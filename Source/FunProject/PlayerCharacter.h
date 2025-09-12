// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "PlayerCharacter.generated.h"

class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UInteractComponent;
class UHealthComponent;
class UHungerComponent;
class UThirstComponent;


UCLASS()
class FUNPROJECT_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// === Components ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* FirstPersonCamera;

	// === Enhanced Input ===
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* IA_Move = nullptr;
	UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* IA_Look = nullptr;
	UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* IA_Jump = nullptr;
	UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* IA_Sprint = nullptr;
	UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* IA_Crouch = nullptr;
	UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* IA_Interact = nullptr;

	// === Input Callbacks ===
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	void StartJump();
	void StopJump();

	void StartSprint();
	void StopSprint();

	void ToggleCrouch();

	// === Movement Variables ===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float CrouchSpeed = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float WalkSpeed = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float SprintSpeed = 650.f;

	// === Character Stats ===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Stats")
	float MaxStamina = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Stats")
	float Stamina = MaxStamina;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Stats")
	float StaminaDrainPerSecond = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Stats")
	float StaminaRegenPerSecond = 5.f;

	bool bWantsToSprint = false;

	void HandleStamina(float DeltaTime);

	// === Components ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInteractComponent* InteractComponent;

	void HandleInteract();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHungerComponent* HungerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UThirstComponent* ThirstComponent;

	// === OnDeath ===
	UFUNCTION()
	void OnDied(UHealthComponent* Comp, AActor* KilledActor);
};
