// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/ItemFragment.h"
#include "ItemFragment_Durability.generated.h"


UCLASS(BlueprintType, EditInlineNew)
class UItemFragment_Durability : public UItemFragment
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Durability", meta=(ClampMin = "0"))
	int32 MaxDurability = 100;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Durability")
	bool bBreakOnZero = true;
	
	/** Runtime */
	UPROPERTY(BlueprintReadOnly, Category = "Durability")
	int32 CurrentDurability = MaxDurability;
	
	UFUNCTION(BlueprintCallable, Category="Durability")
	bool CanUse() const { return CurrentDurability > 0; }
	
	UFUNCTION(BlueprintCallable, Category="Durability")
	bool ApplyUse(int32 Cost)
	{
		CurrentDurability = FMath::Max(0, CurrentDurability - Cost);
		return (CurrentDurability == 0) && bBreakOnZero; // true => Item bricht
	}
	
	virtual void InitializeFragment_Implementation() override
	{
		CurrentDurability = FMath::Clamp(CurrentDurability, 0, MaxDurability);
	}
};
