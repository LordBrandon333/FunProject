#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Delegates/DelegateCombinations.h"
#include "GameplayTagContainer.h"
#include "StaminaComponent.generated.h"

class ACharacter;
class UCharacterMovementComponent;
class UHungerComponent;
class UThirstComponent;
class UStatusEffectsComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnStaminaChanged, class UStaminaComponent*, StaminaComp, float, OldValue, float, NewValue, float, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExhausted, class UStaminaComponent*, StaminaComp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRested, class UStaminaComponent*, StaminaComp);

UCLASS(ClassGroup = (Survival), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class FUNPROJECT_API UStaminaComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UStaminaComponent();

    // === Read API ===
    UFUNCTION(BlueprintPure, Category = "Stamina") float GetStamina() const { return Stamina; }
    UFUNCTION(BlueprintPure, Category = "Stamina") float GetMaxStamina() const { return MaxStamina; }
    UFUNCTION(BlueprintPure, Category = "Stamina") float GetStaminaNormalized() const { return (MaxStamina > 0.f) ? Stamina / MaxStamina : 0.f; }
    UFUNCTION(BlueprintPure, Category = "Stamina") bool  IsSprinting() const { return bIsSprinting; }
    UFUNCTION(BlueprintPure, Category = "Stamina") bool  IsExhausted() const { return bExhausted; }

    // === Control API ===
    UFUNCTION(BlueprintCallable, Category = "Stamina") void RequestSprint(bool bEnable);

    // === Delegates ===
    UPROPERTY(BlueprintAssignable, Category = "Stamina|Events") FOnStaminaChanged OnStaminaChanged;
    UPROPERTY(BlueprintAssignable, Category = "Stamina|Events") FOnExhausted     OnExhausted;
    UPROPERTY(BlueprintAssignable, Category = "Stamina|Events") FOnRested        OnRested;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void UpdateStamina(float DeltaTime);
    bool CanSprint() const;
    bool HasMoveInput() const;
    void ApplyWalkSpeed(float NewSpeed) const;

protected:
    // Values
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (ClampMin = "1.0"))
    float MaxStamina = 100.f;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stamina")
    float Stamina = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina|Rates", meta = (ClampMin = "0.0"))
    float DrainPerSecond = 20.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina|Rates", meta = (ClampMin = "0.0"))
    float RegenPerSecond = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina|Rules", meta = (ClampMin = "0.0"))
    float MinStaminaToStartSprint = 5.f;

    // Movement
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
    float WalkSpeed = 400.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
    float SprintSpeed = 650.f;

    // Rules
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rules")
    bool bRequireMoveInputToSprint = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rules")
    bool bBlockSprintWhenCrouched = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rules")
    bool bBlockSprintInAir = false;

    // Needs coupling
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Needs")
    bool bUseNeeds = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Needs", meta = (EditCondition = "bUseNeeds"))
    float RegenMult_HungerCritical = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Needs", meta = (EditCondition = "bUseNeeds"))
    float RegenMult_ThirstCritical = 0.35f;

    // Tag for Stamina-Regeneration
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Tags", meta = (Categories = "Stat"))
    FGameplayTag StaminaRegenTag;

private:
    // Caches
    TWeakObjectPtr<ACharacter> OwnerChar;
    TWeakObjectPtr<UCharacterMovementComponent> MoveComp;
    TWeakObjectPtr<UHungerComponent> Hunger;
    TWeakObjectPtr<UThirstComponent> Thirst;
    TWeakObjectPtr<UStatusEffectsComponent> Effects;

    bool bWantsSprint = false;
    bool bIsSprinting = false;
    bool bExhausted = false;
};
