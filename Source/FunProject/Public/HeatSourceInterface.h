// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HeatSourceInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UHeatSourceInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class FUNPROJECT_API IHeatSourceInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Temperatre")
	float GetHeatContributionAt(const FVector& WorldLocation) const;
};
