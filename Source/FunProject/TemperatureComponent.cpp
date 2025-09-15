#include "TemperatureComponent.h"
#include "TemperatureManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "WeatherManager.h"
#include "TemperatureZoneVolume.h"

UTemperatureComponent::UTemperatureComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = UpdateInterval;
}

void UTemperatureComponent::BeginPlay()
{
    Super::BeginPlay();

    if (UWorld* W = GetWorld())
    {
        Manager = W->GetSubsystem<UTemperatureManager>();
    }

    // Initiale Caches + Start-Broadcasts (damit HUD sofort Werte hat)
    if (AActor* Owner = GetOwner())
    {
        const FVector Loc = Owner->GetActorLocation();

        if (Manager.IsValid())
        {
            const float AmbientC = Manager->GetAmbientTemperatureC(Loc);
            const float HeatC = Manager->ComputeHeatSourceContributionC(Loc);

            LastAmbientC = AmbientC;
            LastHeatContributionC = HeatC;

            OnAmbientEffectiveChanged.Broadcast(AmbientC, HeatC);
        }
        else
        {
            LastAmbientC = 20.f;
            LastHeatContributionC = 0.f;
            OnAmbientEffectiveChanged.Broadcast(LastAmbientC, LastHeatContributionC);
        }

        OnCoreTempChanged.Broadcast(CoreTempC);
    }
}

void UTemperatureComponent::TickComponent(float Dt, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(Dt, TickType, ThisTickFunction);
    if (Dt <= KINDA_SMALL_NUMBER) return;

    if (!Manager.IsValid())
    {
        if (UWorld* W = GetWorld())
        {
            Manager = W->GetSubsystem<UTemperatureManager>();
        }
        if (!Manager.IsValid()) return;
    }

    AActor* Owner = GetOwner();
    if (!Owner) return;

    if (bUseAdaptiveTickInterval)
    {
        // Distanz in cm (UE ist cm), 3000 cm = 30 m
        const APawn* PlayerPawn = GetWorld() ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr;
        float DesiredInterval = UpdateIntervalNear;

        if (PlayerPawn && Owner != PlayerPawn) // Spieler selbst möglichst "nah" lassen
        {
            const float Dist2 = FVector::DistSquared(PlayerPawn->GetActorLocation(), Owner->GetActorLocation());
            if (Dist2 > DistFarCm * DistFarCm)
                DesiredInterval = UpdateIntervalFar;
            else if (Dist2 > DistNearCm * DistNearCm)
                DesiredInterval = UpdateIntervalMid;
            // sonst Near
        }

        if (!FMath::IsNearlyEqual(PrimaryComponentTick.TickInterval, DesiredInterval, KINDA_SMALL_NUMBER))
            PrimaryComponentTick.TickInterval = DesiredInterval;
    }

    const FVector Loc = Owner->GetActorLocation();

    // Basis + Heat (beides bereits zonen-modifiziert im Manager)
    const float AmbientC = Manager->GetAmbientTemperatureC(Loc);
    const float HeatC = Manager->ComputeHeatSourceContributionC(Loc);

    // Zonen-Infos (für Wetter & evtl. UI)
    const FTemperatureZoneModifiers Z = Manager->GetCombinedZoneModifiers(Loc);

    // Wetter samplen
    float WindMS = 0.f, Rain01 = 0.f;
    if (UWorld* W = GetWorld())
    {
        if (UWeatherManager* WM = W->GetSubsystem<UWeatherManager>())
        {
            const FWeatherSample S = WM->Sample(Loc, Z.bIsIndoors, Z.WindScale);
            WindMS = S.WindMS;
            Rain01 = S.Rain01;
        }
    }

    // Wetness updaten (im Rahmen der Component; optional kannst du’s an ein eigenes Need koppeln)
    if (const UTemperatureRules* R = Manager->GetRules())
    {
        const float DryBoost = 1.f + 0.2f * WindMS; // Wind trocknet schneller
        Wetness01 = FMath::Clamp(
            Wetness01 + Rain01 * R->RainWetnessGainPerSec * Dt
            - R->DryingRatePerSec * DryBoost * Dt,
            0.f, 1.f);
    }

    // Operative Umgebung (Heat gedämpft durch Kleidung)
    const UTemperatureRules* R = Manager->GetRules();
    const float W = R ? R->RadiantToOperativeWeight : 0.35f;
    const float K = R ? R->RadiantInsulationFactor : 0.6f;
    const float CLO = FMath::Max(0.f, InsulationClo);

    float RadiantAdd = HeatC * W / (1.f + K * CLO);
    if (R) RadiantAdd = FMath::Clamp(RadiantAdd, 0.f, R->MaxRadiantAddC);
    const float OperativeAmbientC = AmbientC + RadiantAdd;

    // Effektive Isolierung sinkt bei Wind (höhere Konvektion)
    const float WindIsolationDiv = (R ? (1.f + R->WindChillCoefficient * WindMS) : 1.f);
    const float EffectiveInsulation = InsulationClo / FMath::Max(0.1f, WindIsolationDiv);

    // Caches für HUD
    LastAmbientC = AmbientC;
    LastHeatContributionC = HeatC;
    LastOperativeAmbientC = OperativeAmbientC;

    // Körper mit *operativer* Umgebung und wind-bereinigter Isolierung fortschreiben
    Manager->ComputeBodyStep(CoreTempC, SkinTempC, EffectiveInsulation, Metabolic01, Wetness01, OperativeAmbientC, Dt);

    // Events
    if (!FMath::IsNearlyEqual(CoreTempC, LastBroadcastCore, 0.01f))
    {
        LastBroadcastCore = CoreTempC;
        OnCoreTempChanged.Broadcast(CoreTempC);
    }

    const float EffectiveAmbientC = AmbientC + HeatC;

    if (!FMath::IsNearlyEqual(EffectiveAmbientC, LastSentEffectiveAmbient, AmbientBroadcastEpsilon))
    {
        LastSentEffectiveAmbient = EffectiveAmbientC;
        OnAmbientEffectiveChanged.Broadcast(AmbientC, HeatC);
    }
}
