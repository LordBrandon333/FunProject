#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventorySlotWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UBorder;
class UItemData;
class UInventoryComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotClicked, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSlotMouseDown, int32, SlotIndex, FKey, Button, bool, bShiftDown);

UCLASS(BlueprintType)
class FUNPROJECT_API UInventorySlotWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
    void Setup(UInventoryComponent* InInventory, int32 InSlotIndex, UItemData* InItem, int32 InCount);

    UPROPERTY(BlueprintAssignable, Category = "Inventory|UI")
    FOnSlotClicked OnSlotClicked;

    UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
    void SetSelected(bool bInSelected);

    UFUNCTION(BlueprintPure, Category = "Inventory|UI")
    int32 GetSlotIndex() const { return SlotIndex; }

    UPROPERTY(BlueprintAssignable, Category = "Inventory|UI")
    FOnSlotMouseDown OnSlotMouseDown;

protected:
    virtual void NativeConstruct() override;
    virtual FReply NativeOnPreviewMouseButtonDown(const FGeometry& G, const FPointerEvent& E) override;

    UPROPERTY(meta = (BindWidgetOptional)) UButton* ClickButton = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UImage* Icon = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* CountText = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UBorder* SelectionBorder = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|UI")
    FLinearColor SelectedTint = FLinearColor(0.2f, 0.6f, 1.f, 0.35f);

private:
    UPROPERTY() TObjectPtr<UInventoryComponent> Inventory = nullptr;
    UPROPERTY() TObjectPtr<UItemData> Item = nullptr;
    int32 SlotIndex = INDEX_NONE;
    int32 Count = 0;
    bool  bSelected = false;

    UFUNCTION() void HandleClicked();

    void RefreshVisuals();
};
