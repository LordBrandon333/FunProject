// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/ItemFragment.h"
#include "GameplayTagContainer.h"
#include "ItemFragment_Tool.generated.h"


UCLASS(BlueprintType, EditInlineNew)
class UItemFragment_Tool : public UItemFragment
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tool")
	FGameplayTagContainer EffectiveAgainst; // Resource.Rock, Resource.Ore
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tool", meta=(ClampMin="0.1"))
	float HarvestPower = 1.f;
	
	UFUNCTION(BlueprintCallable, Category="Tool")
	bool CanHit(const FGameplayTagContainer& TargetTags) const
	{
		return TargetTags.HasAny(EffectiveAgainst);
	}
};
