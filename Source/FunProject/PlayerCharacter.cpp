// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "InteractComponent.h"

// Constructor
APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Capsule Init
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	// Camera Setup
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));

	//Movement Defaults
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = true;

	MoveComp->RotationRate = FRotator(0.f, 540.f, 0.f);
	MoveComp->JumpZVelocity = 420.f;
	MoveComp->AirControl = 0.35f;
	MoveComp->MaxWalkSpeed = WalkSpeed;
	MoveComp->MaxWalkSpeedCrouched = CrouchSpeed;
	MoveComp->NavAgentProps.bCanCrouch = true;

	//Interact Component
	InteractComponent = CreateDefaultSubobject<UInteractComponent>(TEXT("InteractComponent"));
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Activate Mapping Context
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsys = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (DefaultMappingContext)
				{
					Subsys->AddMappingContext(DefaultMappingContext, 0);
				}
			}
		}
	}

	Stamina = MaxStamina;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	HandleStamina(DeltaTime);
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (IA_Move) EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		if (IA_Look) EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		if (IA_Jump)
		{
			EIC->BindAction(IA_Jump, ETriggerEvent::Started, this, &APlayerCharacter::StartJump);
			EIC->BindAction(IA_Jump, ETriggerEvent::Completed, this, &APlayerCharacter::StopJump);
			EIC->BindAction(IA_Jump, ETriggerEvent::Canceled, this, &APlayerCharacter::StopJump);
		}
		if (IA_Sprint)
		{
			EIC->BindAction(IA_Sprint, ETriggerEvent::Started, this, &APlayerCharacter::StartSprint);
			EIC->BindAction(IA_Sprint, ETriggerEvent::Completed, this, &APlayerCharacter::StopSprint);
			EIC->BindAction(IA_Sprint, ETriggerEvent::Canceled, this, &APlayerCharacter::StopSprint);
		}
		if (IA_Crouch) EIC->BindAction(IA_Crouch, ETriggerEvent::Started, this, &APlayerCharacter::ToggleCrouch);
		if (IA_Interact) EIC->BindAction(IA_Interact, ETriggerEvent::Started, this, &APlayerCharacter::HandleInteract);
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();
	if (!Controller) return;

	const FRotator YawRot(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FVector ForwardDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
	const FVector RightDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDir, Input.Y);
	AddMovementInput(RightDir, Input.X);
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();
	AddControllerYawInput(Input.X);
	AddControllerPitchInput(Input.Y);
}

void APlayerCharacter::StartJump()
{
	Jump();
}

void APlayerCharacter::StopJump()
{
	StopJumping();
}

void APlayerCharacter::StartSprint()
{
	bWantsToSprint = true;
}

void APlayerCharacter::StopSprint()
{
	bWantsToSprint = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void APlayerCharacter::ToggleCrouch()
{
	if (bIsCrouched) UnCrouch(); else Crouch();
}

void APlayerCharacter::HandleStamina(float DeltaTime)
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();

	const bool bHasMoveInput = MoveComp->GetCurrentAcceleration().SizeSquared() > KINDA_SMALL_NUMBER;
	const bool bCanReallySprint = bWantsToSprint && (Stamina > 1.f) && !bIsCrouched && MoveComp->IsMovingOnGround() && bHasMoveInput;
	if (bCanReallySprint)
	{
		MoveComp->MaxWalkSpeed = SprintSpeed;
		Stamina = FMath::Clamp(Stamina - StaminaDrainPerSecond * DeltaTime, 0.f, MaxStamina);
		if (Stamina <= 0.f)
		{
			bWantsToSprint = false;
			MoveComp->MaxWalkSpeed = WalkSpeed;
		}
	}
	else
	{
		MoveComp->MaxWalkSpeed = WalkSpeed;

		if (!MoveComp->IsFalling())
		{
			Stamina = FMath::Clamp(Stamina + StaminaRegenPerSecond * DeltaTime, 0.f, MaxStamina);
		}
	}
}

void APlayerCharacter::HandleInteract()
{
	if (InteractComponent) InteractComponent->TryInteract();
}
