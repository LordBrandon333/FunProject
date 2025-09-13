#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Delegates/DelegateCombinations.h"
#include "GameplayTagContainer.h"
#include "StatusEffectData.h"
#include "StatRegistry.h"
#include "StatusEffectsComponent.generated.h"

class UStatusEffectData;
class UStatRegistry;

USTRUCT()
struct FActiveEffect
{
    GENERATED_BODY()

    UPROPERTY() UStatusEffectData* Data = nullptr;
    UPROPERTY() float TimeRemaining = 0.f;
    UPROPERTY() int32 Stacks = 1;
    UPROPERTY() FName SourceId = NAME_None;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEffectApplied, UStatusEffectData*, Effect);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEffectRemoved, UStatusEffectData*, Effect);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEffectsChanged);

UCLASS(ClassGroup = (Survival), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class FUNPROJECT_API UStatusEffectsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UStatusEffectsComponent();

    UFUNCTION(BlueprintCallable, Category = "StatusEffects")
    bool ApplyEffect(UStatusEffectData* EffectData, FName SourceId = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "StatusEffects")
    int32 RemoveEffectsByTag(FGameplayTag EffectTag);

    UFUNCTION(BlueprintCallable, Category = "StatusEffects")
    int32 RemoveEffectsBySource(FName InSourceId);

    UFUNCTION(BlueprintPure, Category = "StatusEffects")
    void GetAggregateForStat(const FGameplayTag& StatTag, float& OutAdd, float& OutMul) const;

    UFUNCTION(BlueprintPure, Category = "StatusEffects")
    float ApplyToBase(const FGameplayTag& StatTag, float BaseValue) const;

    UFUNCTION(BlueprintPure, Category = "StatusEffects")
    bool HasEffectTag(FGameplayTag EffectTag) const;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StatusEffects")
    UStatRegistry* StatRegistry = nullptr;

    // === Delegates ===
    UPROPERTY(BlueprintAssignable, Category = "StatusEffects|Events") FOnEffectApplied  OnEffectApplied;
    UPROPERTY(BlueprintAssignable, Category = "StatusEffects|Events") FOnEffectRemoved  OnEffectRemoved;
    UPROPERTY(BlueprintAssignable, Category = "StatusEffects|Events") FOnEffectsChanged OnEffectsChanged;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    UPROPERTY()
    TArray<FActiveEffect> Active;

    void TickEffects(float DeltaTime);
};
