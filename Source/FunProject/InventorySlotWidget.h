#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventorySlotWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UItemData;
class UInventoryComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotClicked, int32, SlotIndex);

UCLASS(BlueprintType)
class FUNPROJECT_API UInventorySlotWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Inventory | UI")
    void Setup(UInventoryComponent* InInventory, int32 InSlotIndex, UItemData* InItem, int32 InCount);

    UPROPERTY(BlueprintAssignable, Category = "Inventory | UI")
    FOnSlotClicked OnSlotClicked;

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidgetOptional)) UButton* ClickButton = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UImage* Icon = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* CountText = nullptr;

private:
    UPROPERTY() TObjectPtr<UInventoryComponent> Inventory = nullptr;
    UPROPERTY() TObjectPtr<UItemData> Item = nullptr;
    int32 SlotIndex = INDEX_NONE;
    int32 Count = 0;

    UFUNCTION() void HandleClicked();

    void RefreshVisuals();
};
