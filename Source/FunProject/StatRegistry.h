#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "StatRegistry.generated.h"

UENUM(BlueprintType)
enum class EStatOp : uint8
{
    Add UMETA(DisplayName = "Add"),
    Mul UMETA(DisplayName = "Multiply"),
};

USTRUCT(BlueprintType)
struct FStatDefaults
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EStatOp DefaultOp = EStatOp::Add;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float DefaultValue = 0.f;
};

UCLASS(BlueprintType)
class FUNPROJECT_API UStatRegistry : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TMap<FGameplayTag, FStatDefaults> Stats;

    UFUNCTION(BlueprintPure)
    bool GetDefaults(const FGameplayTag& Tag, FStatDefaults& OutDefaults) const;
};
