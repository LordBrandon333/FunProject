// HeatSourceComponent.cpp
#include "HeatSourceComponent.h"
#include "TemperatureManager.h"
#include "Engine/World.h"

void UHeatSourceComponent::OnRegister()
{
    Super::OnRegister();
    if (UWorld* W = GetWorld())
    {
        if (auto* Mgr = W->GetSubsystem<UTemperatureManager>())
        {
            Mgr->RegisterHeatSource(this);
        }
    }
}

void UHeatSourceComponent::OnUnregister()
{
    if (UWorld* W = GetWorld())
    {
        if (auto* Mgr = W->GetSubsystem<UTemperatureManager>())
        {
            Mgr->UnregisterHeatSource(this);
        }
    }
    Super::OnUnregister();
}
