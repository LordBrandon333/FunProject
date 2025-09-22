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
//#include "InventoryComponent.h"
#include "TemperatureComponent.h"
#include "HUDWidget.h"
//#include "InventoryWidget.h"
//#include "HotbarWidget.h"
#include "Blueprint/UserWidget.h"

// Constructor
APlayerCharacter::APlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule + Camera
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);
    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
    FirstPersonCamera->bUsePawnControlRotation = true;
    FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));

    // Movement defaults
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    MoveComp->bOrientRotationToMovement = false;
    bUseControllerRotationYaw = true;
    MoveComp->RotationRate = FRotator(0.f, 540.f, 0.f);
    MoveComp->JumpZVelocity = 420.f;
    MoveComp->AirControl = 0.35f;
    MoveComp->MaxWalkSpeed = WalkSpeed;
    MoveComp->MaxWalkSpeedCrouched = CrouchSpeed;
    MoveComp->NavAgentProps.bCanCrouch = true;

    // Components
    InteractComponent = CreateDefaultSubobject<UInteractComponent>(TEXT("InteractComponent"));
    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
    HungerComponent = CreateDefaultSubobject<UHungerComponent>(TEXT("HungerComponent"));
    ThirstComponent = CreateDefaultSubobject<UThirstComponent>(TEXT("ThirstComponent"));
    StaminaComponent = CreateDefaultSubobject<UStaminaComponent>(TEXT("StaminaComponent"));
    //InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
    TemperatureComponent = CreateDefaultSubobject<UTemperatureComponent>(TEXT("TemperatureComponent"));
}

void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Input Mapping Context aktivieren
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

        // HUD
        if (HUDWidgetClass)
        {
            HUDWidget = CreateWidget<UHUDWidget>(PC, HUDWidgetClass);
            if (HUDWidget)
            {
                HUDWidget->AddToViewport(10);
                HUDWidget->InitializeFromCharacter(this);
            }
        }

        // Hotbar
        /*if (HotbarWidgetClass)
        {
            HotbarWidget = CreateWidget<UHotbarWidget>(PC, HotbarWidgetClass);
            if (HotbarWidget)
            {
                HotbarWidget->AddToViewport(5);
                if (InventoryComponent) HotbarWidget->InitializeForInventory(InventoryComponent);
                // Range aus Component �bernehmen
                HotbarWidget->SetHotbarRange(0, InventoryComponent ? InventoryComponent->GetHotbarSize() : 4);

            }
        }

        // Inventory UI
        if (InventoryWidgetClass)
        {
            InventoryWidget = CreateWidget<UInventoryWidget>(PC, InventoryWidgetClass);
            if (InventoryWidget)
            {
                InventoryWidget->AddToViewport(20);
                if (InventoryComponent) InventoryWidget->InitializeForInventory(InventoryComponent);

                const int32 FirstInv = InventoryComponent ? InventoryComponent->GetFirstInventoryIndex() : 4;
                const int32 Total = InventoryComponent ? InventoryComponent->GetCapacity() : 24;
                InventoryWidget->SetRange(FirstInv, FMath::Max(0, Total - FirstInv));
                InventoryWidget->SetVisible(false);
            }
        }

        if (HotbarWidget && InventoryWidget) HotbarWidget->SetInventoryUI(InventoryWidget);*/
    }

    if (HealthComponent)
    {
        HealthComponent->OnDeath.AddDynamic(this, &APlayerCharacter::OnDied);
    }
}

void APlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (IA_Move)   EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
        if (IA_Look)   EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
        if (IA_Jump) {
            EIC->BindAction(IA_Jump, ETriggerEvent::Started, this, &APlayerCharacter::StartJump);
            EIC->BindAction(IA_Jump, ETriggerEvent::Completed, this, &APlayerCharacter::StopJump);
            EIC->BindAction(IA_Jump, ETriggerEvent::Canceled, this, &APlayerCharacter::StopJump);
        }
        if (IA_Sprint) {
            EIC->BindAction(IA_Sprint, ETriggerEvent::Started, this, &APlayerCharacter::StartSprint);
            EIC->BindAction(IA_Sprint, ETriggerEvent::Completed, this, &APlayerCharacter::StopSprint);
            EIC->BindAction(IA_Sprint, ETriggerEvent::Canceled, this, &APlayerCharacter::StopSprint);
        }
        if (IA_Crouch) EIC->BindAction(IA_Crouch, ETriggerEvent::Started, this, &APlayerCharacter::ToggleCrouch);
        if (IA_Interact) EIC->BindAction(IA_Interact, ETriggerEvent::Started, this, &APlayerCharacter::HandleInteract);

        /*
        // Inventory / Hotbar
        if (IA_ToggleInventory) EIC->BindAction(IA_ToggleInventory, ETriggerEvent::Started, this, &APlayerCharacter::ToggleInventory);
        if (IA_PrimaryUse)      EIC->BindAction(IA_PrimaryUse, ETriggerEvent::Started, this, &APlayerCharacter::PrimaryUse);
        if (IA_HotbarScroll)    EIC->BindAction(IA_HotbarScroll, ETriggerEvent::Triggered, this, &APlayerCharacter::HotbarScroll);

        if (IA_Select1) EIC->BindAction(IA_Select1, ETriggerEvent::Started, this, &APlayerCharacter::Select1);
        if (IA_Select2) EIC->BindAction(IA_Select2, ETriggerEvent::Started, this, &APlayerCharacter::Select2);
        if (IA_Select3) EIC->BindAction(IA_Select3, ETriggerEvent::Started, this, &APlayerCharacter::Select3);
        if (IA_Select4) EIC->BindAction(IA_Select4, ETriggerEvent::Started, this, &APlayerCharacter::Select4);
        if (IA_Select5) EIC->BindAction(IA_Select5, ETriggerEvent::Started, this, &APlayerCharacter::Select5);
        if (IA_Select6) EIC->BindAction(IA_Select6, ETriggerEvent::Started, this, &APlayerCharacter::Select6);
        if (IA_Select7) EIC->BindAction(IA_Select7, ETriggerEvent::Started, this, &APlayerCharacter::Select7);
        if (IA_Select8) EIC->BindAction(IA_Select8, ETriggerEvent::Started, this, &APlayerCharacter::Select8);
        if (IA_Select9) EIC->BindAction(IA_Select9, ETriggerEvent::Started, this, &APlayerCharacter::Select9);
        if (IA_Select0) EIC->BindAction(IA_Select0, ETriggerEvent::Started, this, &APlayerCharacter::Select0);
        */
    }
}

// Movement/Input
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

void APlayerCharacter::StartJump() { Jump(); }
void APlayerCharacter::StopJump() { StopJumping(); }
void APlayerCharacter::StartSprint() { if (StaminaComponent) StaminaComponent->RequestSprint(true); }
void APlayerCharacter::StopSprint() { if (StaminaComponent) StaminaComponent->RequestSprint(false); }
void APlayerCharacter::ToggleCrouch() { if (bIsCrouched) UnCrouch(); else Crouch(); }
void APlayerCharacter::HandleInteract() { if (InteractComponent) InteractComponent->TryInteract(); }

// UI/Hotbar
/*
void APlayerCharacter::ToggleInventory()
{
    /*
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

void APlayerCharacter::PrimaryUse()
{
    if (!InventoryComponent) return;

    if (InventoryWidget && (InventoryWidget->GetVisibility() == ESlateVisibility::Visible ||
        InventoryWidget->GetVisibility() == ESlateVisibility::SelfHitTestInvisible))
    {
        return; // im Inventar ge�ffnet nicht benutzen
    }

    InventoryComponent->UseSelectedHotbarItem();
}

void APlayerCharacter::HotbarScroll(const FInputActionValue& Value)
{
    if (!InventoryComponent) return;
    const float Axis = Value.Get<float>();
    if (Axis > 0.1f)       InventoryComponent->OffsetSelectedHotbarIndex(+1);
    else if (Axis < -0.1f) InventoryComponent->OffsetSelectedHotbarIndex(-1);
}

void APlayerCharacter::SelectHotbarIndex(int32 Index)
{
    if (InventoryComponent) InventoryComponent->SetSelectedHotbarIndex(Index);
}
void APlayerCharacter::Select1() { SelectHotbarIndex(0); }
void APlayerCharacter::Select2() { SelectHotbarIndex(1); }
void APlayerCharacter::Select3() { SelectHotbarIndex(2); }
void APlayerCharacter::Select4() { SelectHotbarIndex(3); }
void APlayerCharacter::Select5() { SelectHotbarIndex(4); }
void APlayerCharacter::Select6() { SelectHotbarIndex(5); }
void APlayerCharacter::Select7() { SelectHotbarIndex(6); }
void APlayerCharacter::Select8() { SelectHotbarIndex(7); }
void APlayerCharacter::Select9() { SelectHotbarIndex(8); }
void APlayerCharacter::Select0() { SelectHotbarIndex(9); }
*/
void APlayerCharacter::OnDied(UHealthComponent* /*Comp*/, AActor* /*KilledActor*/)
{
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->DisableInput(PC);
    }
}
