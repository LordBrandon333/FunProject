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
class UStaminaComponent;
//class UInventoryComponent;
class UTemperatureComponent;
class UHUDWidget;
//class UInventoryWidget;
//class UHotbarWidget;

UCLASS()
class FUNPROJECT_API APlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    APlayerCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // === Components ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCameraComponent* FirstPersonCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UInteractComponent* InteractComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UHealthComponent* HealthComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UHungerComponent* HungerComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UThirstComponent* ThirstComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaminaComponent* StaminaComponent;

    //UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    //UInventoryComponent* InventoryComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTemperatureComponent* TemperatureComponent;

    // === Enhanced Input ===
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputMappingContext* DefaultMappingContext = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* IA_Move = nullptr;
    UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* IA_Look = nullptr;
    UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* IA_Jump = nullptr;
    UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* IA_Sprint = nullptr;
    UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* IA_Crouch = nullptr;
    UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* IA_Interact = nullptr;

    // NEW: Inventory UI / Hotbar Inputs
    UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* IA_ToggleInventory = nullptr;
    UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* IA_PrimaryUse = nullptr;
    UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* IA_HotbarScroll = nullptr;
    UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* IA_Select1 = nullptr;
    UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* IA_Select2 = nullptr;
    UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* IA_Select3 = nullptr;
    UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* IA_Select4 = nullptr;
    UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* IA_Select5 = nullptr;
    UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* IA_Select6 = nullptr;
    UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* IA_Select7 = nullptr;
    UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* IA_Select8 = nullptr;
    UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* IA_Select9 = nullptr;
    UPROPERTY(EditDefaultsOnly, Category = "Input") UInputAction* IA_Select0 = nullptr;

    // === UI Classes ===
    UPROPERTY(EditDefaultsOnly, Category = "UI") TSubclassOf<UHUDWidget>       HUDWidgetClass;
    //UPROPERTY(EditDefaultsOnly, Category = "UI") TSubclassOf<UInventoryWidget> InventoryWidgetClass;
    //UPROPERTY(EditDefaultsOnly, Category = "UI") TSubclassOf<UHotbarWidget>    HotbarWidgetClass;

    UPROPERTY() UHUDWidget* HUDWidget = nullptr;
    //UPROPERTY() UInventoryWidget* InventoryWidget = nullptr;
    //UPROPERTY() UHotbarWidget* HotbarWidget = nullptr;

    // === Movement Vars ===
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement") float CrouchSpeed = 200.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement") float WalkSpeed = 400.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement") float SprintSpeed = 650.f;

    // === Input Callbacks ===
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void StartJump();
    void StopJump();
    void StartSprint();
    void StopSprint();
    void ToggleCrouch();
    void HandleInteract();

    // UI/Hotbar
    /*
    void ToggleInventory();
    void PrimaryUse();
    void HotbarScroll(const FInputActionValue& Value);
    void SelectHotbarIndex(int32 Index);
    void Select1(); void Select2(); void Select3(); void Select4(); void Select5();
    void Select6(); void Select7(); void Select8(); void Select9(); void Select0();
    */
    // === OnDeath ===
    UFUNCTION() void OnDied(UHealthComponent* Comp, AActor* KilledActor);
};