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
    const float EffectiveAmbientC = AmbientC + HeatC;

    // Caches aktualisieren
    LastAmbientC = AmbientC;
    LastHeatContributionC = HeatC;

    // Körper mit *effektiver* Umgebung fortschreiben
    Manager->ComputeBodyStep(CoreTempC, SkinTempC, InsulationClo, Metabolic01, Wetness01, EffectiveAmbientC, Dt);

    // Core-Event (nur bei Änderung)
    if (!FMath::IsNearlyEqual(CoreTempC, LastBroadcastCore, 0.01f))
    {
        LastBroadcastCore = CoreTempC;
        OnCoreTempChanged.Broadcast(CoreTempC);
    }

    // Ambient-Event (immer senden; optional mit Threshold/Rate-Limit)
    OnAmbientEffectiveChanged.Broadcast(AmbientC, HeatC);
}
