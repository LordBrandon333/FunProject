// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "InteractComponent.h"
#include "HealthComponent.h"
#include "NeedComponent.h"
#include "StaminaComponent.h"
#include "StatusEffectsComponent.h"
#include "InventoryComponent.h"
#include "HUDWidget.h"
#include "InventoryWidget.h"
#include "HotbarWidget.h"
#include "Blueprint/UserWidget.h"


// Constructor
APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

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
	MoveComp->MaxWalkSpeedCrouched = CrouchSpeed;
	MoveComp->NavAgentProps.bCanCrouch = true;

	//Components
	InteractComponent = CreateDefaultSubobject<UInteractComponent>(TEXT("InteractComponent"));
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HungerComponent = CreateDefaultSubobject<UHungerComponent>(TEXT("HungerComponent"));
	ThirstComponent = CreateDefaultSubobject<UThirstComponent>(TEXT("ThirstComponent"));
	StaminaComponent = CreateDefaultSubobject<UStaminaComponent>(TEXT("StaminaComponent"));
	StatusEffectsComponent = CreateDefaultSubobject<UStatusEffectsComponent>(TEXT("StatusEffectsComponent"));
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
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

		if (HUDWidgetClass)
		{
			HUDWidget = CreateWidget<UHUDWidget>(PC, HUDWidgetClass);
			if (HUDWidget)
			{
				HUDWidget->AddToViewport(10);
				HUDWidget->InitializeFromCharacter(this);
			}
		}

		if (HotbarWidgetClass)
		{
			HotbarWidget = CreateWidget<UHotbarWidget>(PC, HotbarWidgetClass);
			if (HotbarWidget)
			{
				HotbarWidget->AddToViewport(5);
				if (InventoryComponent) HotbarWidget->InitializeForInventory(InventoryComponent);
				HotbarWidget->SetHotbarRange(0, 4);
			}
		}

		if (InventoryWidgetClass)
		{
			InventoryWidget = CreateWidget<UInventoryWidget>(PC, InventoryWidgetClass);
			if (InventoryWidget)
			{
				InventoryWidget->AddToViewport(20);
				if (InventoryComponent) InventoryWidget->InitializeForInventory(InventoryComponent);
				InventoryWidget->SetVisible(false);
			}
		}
	}

	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &APlayerCharacter::OnDied);
	}
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
		if (IA_ToggleInventory) EIC->BindAction(IA_ToggleInventory, ETriggerEvent::Started, this, &APlayerCharacter::ToggleInventory);
		// probably change later
		if (IA_UseSlot1) EIC->BindAction(IA_UseSlot1, ETriggerEvent::Started, this, &APlayerCharacter::QuickUse1);
		if (IA_UseSlot2) EIC->BindAction(IA_UseSlot2, ETriggerEvent::Started, this, &APlayerCharacter::QuickUse2);
		if (IA_UseSlot3) EIC->BindAction(IA_UseSlot3, ETriggerEvent::Started, this, &APlayerCharacter::QuickUse3);
		if (IA_UseSlot4) EIC->BindAction(IA_UseSlot4, ETriggerEvent::Started, this, &APlayerCharacter::QuickUse4);
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
	if (StaminaComponent) StaminaComponent->RequestSprint(true);
}

void APlayerCharacter::StopSprint()
{
	if (StaminaComponent) StaminaComponent->RequestSprint(false);
}

void APlayerCharacter::ToggleCrouch()
{
	if (bIsCrouched) UnCrouch(); else Crouch();
}

void APlayerCharacter::ToggleInventory()
{
	if (!InventoryWidget) return;

	const bool bShow = (InventoryWidget->GetVisibility() != ESlateVisibility::Visible &&
		InventoryWidget->GetVisibility() != ESlateVisibility::SelfHitTestInvisible);

	InventoryWidget->SetVisible(bShow);

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (bShow)
		{
			FInputModeGameAndUI Mode;
			Mode.SetWidgetToFocus(InventoryWidget->TakeWidget());
			Mode.SetHideCursorDuringCapture(false);
			PC->SetInputMode(Mode);
			PC->bShowMouseCursor = true;
		}
		else
		{
			FInputModeGameOnly Mode;
			PC->SetInputMode(Mode);
			PC->bShowMouseCursor = false;
		}
	}
}

void APlayerCharacter::QuickUseSlot(int32 Index)
{
	if (InventoryComponent)
	{
		InventoryComponent->UseSlot(Index);
	}
}

void APlayerCharacter::QuickUse1() { QuickUseSlot(0); }
void APlayerCharacter::QuickUse2() { QuickUseSlot(1); }
void APlayerCharacter::QuickUse3() { QuickUseSlot(2); }
void APlayerCharacter::QuickUse4() { QuickUseSlot(3); }

void APlayerCharacter::HandleInteract()
{
	if (InteractComponent) InteractComponent->TryInteract();
}

void APlayerCharacter::OnDied(UHealthComponent* Comp, AActor* KilledActor)
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->DisableInput(PC);
	}
	// TODO: Ragdoll, Respawn, UI, etc.
}
