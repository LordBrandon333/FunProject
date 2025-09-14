#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWidget.generated.h"

class UUniformGridPanel;
class UCanvasPanel;
class UInventoryComponent;
class UInventorySlotWidget;
class UCursorStackWidget;
class UItemData;

UCLASS(BlueprintType)
class FUNPROJECT_API UInventoryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
    void InitializeForInventory(UInventoryComponent* InInventory);

    UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
    void SetVisible(bool bShow);

    UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
    void SetCursorEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
    void SetRange(int32 InStartIndex, int32 InCount);

    UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
    void HandleSlotMouseDown(int32 SlotIndex, FKey Button, bool bShift);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UPROPERTY(meta = (BindWidget)) UUniformGridPanel* ItemsGrid = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UCanvasPanel* RootCanvas = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|UI")
    TSubclassOf<UInventorySlotWidget> SlotWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|UI")
    TSubclassOf<UCursorStackWidget> CursorWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|UI", meta = (ClampMin = "1"))
    int32 Columns = 6;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|UI", meta = (ClampMin = "0"))
    int32 StartIndex = 4;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|UI", meta = (ClampMin = "0"))
    int32 VisibleCount = 24;

private:
    UPROPERTY() TObjectPtr<UInventoryComponent> Inventory = nullptr;

    UPROPERTY() TObjectPtr<UCursorStackWidget> CursorVisual = nullptr;
    UPROPERTY() TObjectPtr<UItemData>          CursorItem = nullptr;
    int32 CursorCount = 0;

    UFUNCTION() void OnInventoryChanged();
    UFUNCTION() void OnSlotMouseDown(int32 SlotIndex, FKey Button, bool bShift);

    void RebuildGrid();
    void ClearGrid();

    // Cursor helpers
    void CursorClear();
    void CursorPickup(UItemData* Item, int32 Count);
    void CursorPlaceIntoSlot(int32 SlotIndex, bool bSingleUnit);
};
