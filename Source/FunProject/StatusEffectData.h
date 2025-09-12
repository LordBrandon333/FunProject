// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "StatusEffectData.generated.h"

UENUM(BlueprintType)
enum class EStatOp : uint8
{
	Add UMETA(DisplayName = "Add"),
	Mul UMETA(DisplayName = "Multiply"),
};

USTRUCT(BlueprintType)
struct FStatModifier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier")
	FName StatTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier")
	EStatOp Op = EStatOp::Add;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier")
	float Value = 0.f;
};

UENUM(BlueprintType)
enum class EEffectStackingPolicy : uint8
{
	Additive UMETA(DisplayName = "Additive"),
	RefreshDuration UMETA(DisplayName = "Refresh Duration"),
	Exclusive UMETA(DisplayName = "Exclusive"),
};


UCLASS()
class FUNPROJECT_API UStatusEffectData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	FName EffectTag = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect", meta = (ClampMin = "0.1"))
	float Duration = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect", meta = (ClampMin = "1"))
	int32 MaxStacks = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	EEffectStackingPolicy Stacking = EEffectStackingPolicy::RefreshDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	TArray<FStatModifier> Modifiers;
};
