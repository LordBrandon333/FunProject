// Fill out your copyright notice in the Description page of Project Settings.


#include "TemperatureZoneVolume.h"
#include "TemperatureManager.h"
#include "Engine/World.h"

void ATemperatureZoneVolume::BeginPlay()
{
	Super::BeginPlay();
	if (UWorld* W = GetWorld())
	{
		if (auto* Mgr = W->GetSubsystem<UTemperatureManager>())
		{
			Mgr->RegisterZone(this);
		}
	}
}

void ATemperatureZoneVolume::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* W = GetWorld())
	{
		if (auto* Mgr = W->GetSubsystem<UTemperatureManager>())
		{
			Mgr->UnregisterZone(this);
		}
	}
	Super::EndPlay(EndPlayReason);
}

bool ATemperatureZoneVolume::ContainsPoint(const FVector& WorldLocation) const
{
	return EncompassesPoint(WorldLocation, 0.f, nullptr);
}
