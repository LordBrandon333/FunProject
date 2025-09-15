#include "TemperatureComponent.h"
#include "TemperatureManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UTemperatureComponent::UTemperatureComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
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

    const FVector Loc = Owner->GetActorLocation();

    // Basis + Heat getrennt berechnen
    const float AmbientC = Manager->GetAmbientTemperatureC(Loc);
    const float HeatC = Manager->ComputeHeatSourceContributionC(Loc);
    
    const UTemperatureRules* R = Manager->GetRules();
    const float W = R ? R->RadiantToOperativeWeight : 0.35f;
    const float K = R ? R->RadiantInsulationFactor : 0.6f;
    const float CLO = FMath::Max(0.f, InsulationClo);

    // gedämpfter Strahlungsanteil (Kleidung reduziert, und nach oben begrenzen)
    float RadiantAdd = HeatC * W / (1.f + K * CLO);
    if (R) RadiantAdd = FMath::Clamp(RadiantAdd, 0.f, R->MaxRadiantAddC);

    // Operative Temperatur für die Haut
    const float OperativeAmbientC = AmbientC + RadiantAdd;

    // Caches aktualisieren
    LastAmbientC = AmbientC;
    LastHeatContributionC = HeatC;
    LastOperativeAmbientC = OperativeAmbientC;

    // Körper mit *operativer* Umgebung fortschreiben (nicht mit 1:1 Ambient+Heat!)
    Manager->ComputeBodyStep(CoreTempC, SkinTempC, InsulationClo, Metabolic01, Wetness01, OperativeAmbientC, Dt);

    // Events (lassen Ambient & Heat getrennt, damit UI/Debug flexibel bleibt)
    if (!FMath::IsNearlyEqual(CoreTempC, LastBroadcastCore, 0.01f))
    {
        LastBroadcastCore = CoreTempC;
        OnCoreTempChanged.Broadcast(CoreTempC);
    }
    OnAmbientEffectiveChanged.Broadcast(AmbientC, HeatC);
}
