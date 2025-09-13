// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "HealthComponent.generated.h"

class UDamageType;
class UStatusEffectsComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnHealthChanged, UHealthComponent*, HealthComp, float, OldValue, float, NewValue, float, Delta, AActor*, InstigatorActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeath, UHealthComponent*, HealthComp, AActor*, KilledActor);



UCLASS( ClassGroup=(Survival), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class FUNPROJECT_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHealthComponent();

	// === Read API ===
	UFUNCTION(BlueprintPure, Category = "Health") float GetHealth() const { return Health; }
	UFUNCTION(BlueprintPure, Category = "Health") float GetMaxHealth() const { return MaxHealth; }
	UFUNCTION(BlueprintPure, Category = "Health") float GetHealthNormalized() const { return (MaxHealth > 0.f) ? Health / MaxHealth : 0.f; }
	UFUNCTION(BlueprintPure, Category = "Health") bool IsDead() const { return Health <= 0.f; }

	// === Write API ===
	UFUNCTION(BlueprintCallable, Category = "Health") void Heal(float Amount);
	UFUNCTION(BlueprintCallable, Category = "Health") void Kill();
	UFUNCTION(BlueprintCallable, Category = "Health") void SetMaxHealth(float NewMax, bool bClampCurrent = true, bool bKeepPercent = false);

	// === Delegates ===
	UPROPERTY(BlueprintAssignable, Category = "Health | Events") FOnHealthChanged OnHealthChanged;
	UPROPERTY(BlueprintAssignable, Category = "Health | Events") FOnDeath OnDeath;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// === Regeneration ===
	void StartRegen();
	void StopRegen();
	void RegenTick();

	float GetEffectiveHealthRegenRate() const;
	void RefreshRegenTimer();

	UFUNCTION()
	void OnEffectsChanged();

	void BroadcastHealthChanged(float Old, float New, float Delta, AActor* InstigatorActor);

	// === Damage Hooks ===
	UFUNCTION()
	void HandleAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Health");
	float Health = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	bool bAutoRegenEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health | Regen", meta = (EditCondition = "bAutoRegenEnabled", ClampMin = "0.0"))
	float RegenDelay = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health | Regen", meta = (EditCondition = "bAutoRegenEnabled", ClampMin = "0.0"))
	float RegenPerSecond = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health | Regen", meta = (EditCondition = "bAutoRegenEnabled", ClampMin = "0.01", ClampMax = "0.5"))
	float RegenTickInterval = 0.1f;

	UPROPERTY(VisibleInstanceOnly, Category = "Health | Regen")
	double NextAllowedPositiveRegenTime = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health | Tuning", meta = (ClampMin = "0.0"))
	float DamageMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health | Tags", meta = (Categories = "Stat"))
	FGameplayTag DamageReceivedTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health | Tags", meta = (Categories = "Stat"))
	FGameplayTag HealthRegenTag;

private:
	FTimerHandle RegenDelayHandle;
	FTimerHandle RegenTickHandle;
	TWeakObjectPtr<UStatusEffectsComponent> Effects;
};
