#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWidget.generated.h"

class UUniformGridPanel;
class UInventoryComponent;
class UInventorySlotWidget;

UCLASS(BlueprintType)
class FUNPROJECT_API UInventoryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
    void InitializeForInventory(UInventoryComponent* InInventory);

    UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
    void SetVisible(bool bShow);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UPROPERTY(meta = (BindWidget)) UUniformGridPanel* ItemsGrid = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|UI")
    TSubclassOf<UInventorySlotWidget> SlotWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|UI", meta = (ClampMin = "1"))
    int32 Columns = 6;

private:
    UPROPERTY() TObjectPtr<UInventoryComponent> Inventory = nullptr;

    UFUNCTION() void OnInventoryChanged();
    UFUNCTION() void OnSlotClicked(int32 SlotIndex);

    void RebuildGrid();
    void ClearGrid();
};
