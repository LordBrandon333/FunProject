#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HotbarWidget.generated.h"

class UHorizontalBox;
class UInventoryComponent;
class UInventorySlotWidget;

UCLASS(BlueprintType)
class FUNPROJECT_API UHotbarWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Inventory | UI")
    void InitializeForInventory(UInventoryComponent* InInventory);

    UFUNCTION(BlueprintCallable, Category = "Inventory | UI")
    void SetHotbarRange(int32 InStartIndex, int32 InSize);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UPROPERTY(meta = (BindWidget)) UHorizontalBox* HotbarBox = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory | UI")
    TSubclassOf<UInventorySlotWidget> SlotWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory | UI", meta = (ClampMin = "1"))
    int32 HotbarSize = 4;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory | UI", meta = (ClampMin = "0"))
    int32 StartIndex = 0;

private:
    UPROPERTY() TObjectPtr<UInventoryComponent> Inventory = nullptr;

    UFUNCTION() void OnInventoryChanged();
    UFUNCTION() void OnSlotClicked(int32 SlotIndex);

    void RebuildBar();
    void ClearBar();
};
