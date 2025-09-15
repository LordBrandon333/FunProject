#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDWidget.generated.h"

class UProgressBar;
class UTextBlock;
class APlayerCharacter;
class UHealthComponent;
class UStaminaComponent;
class UNeedComponent;
class UTemperatureComponent;

UCLASS(BlueprintType)
class FUNPROJECT_API UHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Call from PlayerCharacter after CreateWidget
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void InitializeFromCharacter(APlayerCharacter* InCharacter);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // --- Bound Widgets (add these in the Widget Blueprint with exact names) ---
    UPROPERTY(meta = (BindWidget)) UProgressBar* HealthBar = nullptr;
    UPROPERTY(meta = (BindWidget)) UProgressBar* StaminaBar = nullptr;
    UPROPERTY(meta = (BindWidget)) UProgressBar* HungerBar = nullptr;
    UPROPERTY(meta = (BindWidget)) UProgressBar* ThirstBar = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* HealthText = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* StaminaText = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* HungerText = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* ThirstText = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* AmbientTempText = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* BodyTempText = nullptr;

private:
    TWeakObjectPtr<APlayerCharacter> Player;
    TWeakObjectPtr<UHealthComponent> Health;
    TWeakObjectPtr<UStaminaComponent> Stamina;
    TWeakObjectPtr<UNeedComponent>   Hunger;
    TWeakObjectPtr<UNeedComponent>   Thirst;
    TWeakObjectPtr<UTemperatureComponent> Temperature;

    void RefreshAll(); // initial fill & fallbacks

    // --- Event handlers (signatures must match your components) ---
    UFUNCTION() void OnHealthChanged(UHealthComponent* Comp, float OldV, float NewV, float Delta, AActor* Instigator);
    UFUNCTION() void OnStaminaChanged(class UStaminaComponent* Comp, float OldV, float NewV, float Delta);
    UFUNCTION() void OnNeedChanged(UNeedComponent* Comp, float OldV, float NewV, float Delta);
    UFUNCTION() void OnAmbientTempChanged(float OldC, float NewC);
    UFUNCTION() void OnBodyTempChanged(float OldC, float NewC);
    
    void UpdateTempTexts();
    void SetBar(UProgressBar* Bar, UTextBlock* Txt, float Current, float Max);
};
