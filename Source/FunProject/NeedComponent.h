// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "NeedComponent.generated.h"

class UStatusEffectsComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnNeedChanged, class UNeedComponent*, NeedComp, float, OldValue, float, NewValue, float, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNeedCritical, class UNeedComponent*, NeedComp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNeedRecovered, class UNeedComponent*, NeedComp);

UCLASS( ClassGroup=(Survival), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent) )
class FUNPROJECT_API UNeedComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UNeedComponent();

	// === Read API ===
	UFUNCTION(BlueprintPure, Category = "Need") float GetCurrent() const { return Current; }
	UFUNCTION(BlueprintPure, Category = "Need") float GetMax() const { return Max; }
	UFUNCTION(BlueprintPure, Category = "Need") float GetNormalized() const { return (Max > 0.f) ? Current / Max : 0.f; }
	UFUNCTION(BlueprintPure, Category = "Need") bool IsCritical() const { return bIsInCritical; }
	UFUNCTION(BlueprintPure, Category = "Need") FName GetStatTag() const { return StatTag; }

	// === Write API ===
	UFUNCTION(BlueprintCallable, Category = "Need") void Add(float Amount);
	UFUNCTION(BlueprintCallable, Category = "Need") void Consume(float Amount);
	UFUNCTION(BlueprintCallable, Category = "Need") void SetMax(float NewMax, bool bClampCurrent = true, bool bKeepPercent = false);

	UFUNCTION(BlueprintCallable, Category = "Need | Runtime") void SetDecayPerSecond(float Value) { DecayPerSecond = FMath::Max(0.f, Value); }
	UFUNCTION(BlueprintCallable, Category = "Need | Runtime") void SetRegenPerSecond(float Value) { RegenPerSecond = FMath::Max(0.f, Value); }
	UFUNCTION(BlueprintCallable, Category = "Need | Runtime") void PauseNeed(bool bPause) { bPause = bPause; }

	UFUNCTION(BlueprintCallable, Category = "Need | Debug") void SetDebugPrint(bool bEnable) { bDebugPrint = bEnable; }

	// === Delegates ===
	UPROPERTY(BlueprintAssignable, Category = "Need | Events") FOnNeedChanged OnNeedChanged;
	UPROPERTY(BlueprintAssignable, Category = "Need | Events") FOnNeedCritical OnCritical;
	UPROPERTY(BlueprintAssignable, Category = "Need | Events") FOnNeedRecovered OnRecovered;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void StartNeedTimer();
	void StopNeedTimer();
	void NeedTick();
	void HandleThresholds(float OldValue, float NewValue);

	float GetEffectiveDecayPerSecond() const;
	float GetEffectiveRegenPerSecond() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Need") 
	FName StatTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Need", meta = (ClampMin = "1.0"))
	float Max = 100.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Need")
	float Current = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Need | Rates", meta = (ClampMin = "0.0"))
	float DecayPerSecond = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Need | Rates", meta = (ClampMin = "0.0"))
	float RegenPerSecond = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Need | Rated", meta = (ClampMin = "0.5", ClampMax = "5.0"))
	float TickInterval = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Need | Thresholds", meta = (ClampMin = "0.0"))
	float CriticalThreshold = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Need | Debug")
	bool bDebugPrint = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Need | Tags", meta = (Categories = "Stat"))
	FGameplayTag DecayTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Need | Tags", meta = (Categories = "Stat"))
	FGameplayTag RegenTag;

private:
	bool bPaused = false;
	bool bIsInCritical = false;
	FTimerHandle TickHandle;
	TWeakObjectPtr<UStatusEffectsComponent> Effects;
};

// == Hunger and Thirst Components (Only Defaults) ===
UCLASS(ClassGroup = (Survival), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class FUNPROJECT_API UHungerComponent : public UNeedComponent
{
	GENERATED_BODY()
public:
	UHungerComponent();
};


UCLASS(ClassGroup = (Survival), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class FUNPROJECT_API UThirstComponent : public UNeedComponent
{
	GENERATED_BODY()
public:
	UThirstComponent();
};
