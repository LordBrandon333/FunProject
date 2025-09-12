// Fill out your copyright notice in the Description page of Project Settings.


#include "StaminaComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NeedComponent.h" 

// Sets default values for this component's properties
UStaminaComponent::UStaminaComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStaminaComponent::RequestSprint(bool bEnable)
{
    bWantsSprint = bEnable;
}

void UStaminaComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerChar = Cast<ACharacter>(GetOwner());
    if (OwnerChar.IsValid())
    {
        MoveComp = OwnerChar->GetCharacterMovement();
        if (MoveComp.IsValid())
        {
            ApplyWalkSpeed(WalkSpeed);
        }
        if (bUseNeeds)
        {
            Hunger = OwnerChar->FindComponentByClass<UHungerComponent>();
            Thirst = OwnerChar->FindComponentByClass<UThirstComponent>();
        }
    }

    Stamina = FMath::Clamp(MaxStamina, 1.f, MaxStamina);
}

void UStaminaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateStamina(DeltaTime);
}

void UStaminaComponent::UpdateStamina(float DeltaTime)
{
    if (!MoveComp.IsValid()) return;

    const bool bCanReallySprint = bWantsSprint && CanSprint();
    const float Old = Stamina;

    if (bCanReallySprint)
    {
        if (!bIsSprinting)
        {
            bIsSprinting = true;
            ApplyWalkSpeed(SprintSpeed);
        }

        Stamina = FMath::Clamp(Stamina - (DrainPerSecond * DeltaTime), 0.f, MaxStamina);

        if (Stamina <= 0.f)
        {
            bExhausted = true;
            bIsSprinting = false;
            bWantsSprint = false;
            ApplyWalkSpeed(WalkSpeed);
            OnExhausted.Broadcast(this);
        }
    }
    else
    {
        if (bIsSprinting)
        {
            bIsSprinting = false;
            ApplyWalkSpeed(WalkSpeed);
        }

        float Regen = MoveComp->IsFalling() ? 0.f : RegenPerSecond;

        if (bUseNeeds)
        {
            if (Hunger.IsValid() && Hunger->IsCritical())  Regen *= RegenMult_HungerCritical;
            if (Thirst.IsValid() && Thirst->IsCritical())  Regen *= RegenMult_ThirstCritical;
        }

        Stamina = FMath::Clamp(Stamina + Regen * DeltaTime, 0.f, MaxStamina);

        if (bExhausted && Stamina >= MinStaminaToStartSprint)
        {
            bExhausted = false;
            OnRested.Broadcast(this);
        }
    }

    if (!FMath::IsNearlyEqual(Old, Stamina))
    {
        OnStaminaChanged.Broadcast(this, Old, Stamina, Stamina - Old);
    }
}

bool UStaminaComponent::CanSprint() const
{
    if (!OwnerChar.IsValid() || !MoveComp.IsValid()) return false;

    if (bBlockSprintWhenCrouched && OwnerChar->bIsCrouched) return false;
    if (bBlockSprintInAir && !MoveComp->IsMovingOnGround()) return false;
    if (bRequireMoveInputToSprint && !HasMoveInput()) return false;
    if (Stamina < MinStaminaToStartSprint) return false;

    return true;
}

bool UStaminaComponent::HasMoveInput() const
{
    if (!MoveComp.IsValid()) return false;
    return MoveComp->GetCurrentAcceleration().SizeSquared() > KINDA_SMALL_NUMBER;
}

void UStaminaComponent::ApplyWalkSpeed(float NewSpeed) const
{
    if (MoveComp.IsValid())
    {
        MoveComp->MaxWalkSpeed = NewSpeed;
    }
}

