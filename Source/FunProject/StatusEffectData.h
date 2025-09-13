#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "StatRegistry.h" 
#include "StatusEffectData.generated.h"

USTRUCT(BlueprintType)
struct FStatModifierRef
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "Stat"))
    FGameplayTag StatTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bOverrideOp = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bOverrideOp"))
    EStatOp Op = EStatOp::Add;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bOverrideValue = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bOverrideValue"))
    float Value = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bScaleByStacks = true;
};

UENUM(BlueprintType)
enum class EEffectStackingPolicy : uint8
{
    Additive         UMETA(DisplayName = "Additive"),       
    RefreshDuration  UMETA(DisplayName = "Refresh Duration"),
    Exclusive        UMETA(DisplayName = "Exclusive"),       
};

UCLASS(BlueprintType)
class FUNPROJECT_API UStatusEffectData : public UDataAsset
{
    GENERATED_BODY()
public:

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Effect"))
    FGameplayTag EffectTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.1"))
    float Duration = 10.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "1"))
    int32 MaxStacks = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    EEffectStackingPolicy Stacking = EEffectStackingPolicy::RefreshDuration;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TArray<FStatModifierRef> Modifiers;
};
