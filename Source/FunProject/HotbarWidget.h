//HotbarWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HotbarWidget.generated.h"

class UHorizontalBox;
class UInventoryComponent;
class UInventorySlotWidget;
class UInventoryWidget;

UCLASS(BlueprintType)
class FUNPROJECT_API UHotbarWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
    void InitializeForInventory(UInventoryComponent* InInventory);

    UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
    void SetHotbarRange(int32 InStartIndex, int32 InSize);

    UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
    void SetInventoryUI(UInventoryWidget* InUI) { InventoryUI = InUI; }

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UPROPERTY(meta = (BindWidget)) UHorizontalBox* HotbarBox = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|UI")
    TSubclassOf<UInventorySlotWidget> SlotWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|UI", meta = (ClampMin = "1"))
    int32 HotbarSize = 4;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|UI", meta = (ClampMin = "0"))
    int32 StartIndex = 0; // Hotbar 0..HotbarSize-1

private:
    UPROPERTY() TObjectPtr<UInventoryComponent> Inventory = nullptr;
    UPROPERTY() TObjectPtr<UInventoryWidget> InventoryUI = nullptr;

    UFUNCTION() void OnInventoryChanged();
    UFUNCTION() void OnHotbarIndexChanged(int32 NewIndex);
    UFUNCTION() void OnSlotClicked(int32 SlotIndex);
    UFUNCTION() void OnSlotMouseDownFromHotbar(int32 SlotIndex, FKey Button, bool bShift);

    void RebuildBar();
    void ClearBar();
    void UpdateSelection();
};
