// TemperatureComponent.cpp
#include "TemperatureComponent.h"
#include "TemperatureManager.h"
#include "Engine/World.h"

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
}

void UTemperatureComponent::TickComponent(float Dt, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(Dt, TickType, ThisTickFunction);
    if (!Manager.IsValid() || Dt <= KINDA_SMALL_NUMBER) return;

    const FVector Loc = GetOwner()->GetActorLocation();

    const float AmbientC = Manager->GetAmbientTemperatureC(Loc);
    const float HeatC = Manager->ComputeHeatSourceContributionC(Loc);
    const float EffectiveAmbientC = AmbientC + HeatC;

    LastAmbientC = AmbientC;
    LastHeatContributionC = HeatC;

    Manager->ComputeBodyStep(CoreTempC, SkinTempC, InsulationClo, Metabolic01, Wetness01, EffectiveAmbientC, Dt);

    // Delegates fürs HUD
    if (!FMath::IsNearlyEqual(CoreTempC, LastBroadcastCore, 0.01f))
    {
        LastBroadcastCore = CoreTempC;
        OnCoreTempChanged.Broadcast(CoreTempC);
    }

    // Ambient: an neues und altes Event feuern
    OnAmbientEffectiveChanged.Broadcast(AmbientC, HeatC);
}
