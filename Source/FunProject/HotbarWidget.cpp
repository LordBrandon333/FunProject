#include "HotbarWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "InventoryComponent.h"
#include "InventorySlotWidget.h"
#include "ItemData.h"

void UHotbarWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UHotbarWidget::NativeDestruct()
{
    if (Inventory)
    {
        Inventory->OnInventoryChanged.RemoveAll(this);
    }
    Super::NativeDestruct();
}

void UHotbarWidget::InitializeForInventory(UInventoryComponent* InInventory)
{
    if (!InInventory) return;

    if (Inventory)
    {
        Inventory->OnInventoryChanged.RemoveAll(this);
    }

    Inventory = InInventory;
    Inventory->OnInventoryChanged.AddDynamic(this, &UHotbarWidget::OnInventoryChanged);

    RebuildBar();
}

void UHotbarWidget::SetHotbarRange(int32 InStartIndex, int32 InSize)
{
    StartIndex = FMath::Max(0, InStartIndex);
    HotbarSize = FMath::Max(1, InSize);
    RebuildBar();
}

void UHotbarWidget::OnInventoryChanged()
{
    RebuildBar();
}

void UHotbarWidget::OnSlotClicked(int32 SlotIndex)
{
    if (!Inventory) return;
    Inventory->UseSlot(SlotIndex);
}

void UHotbarWidget::ClearBar()
{
    if (HotbarBox)
    {
        HotbarBox->ClearChildren();
    }
}

void UHotbarWidget::RebuildBar()
{
    if (!HotbarBox || !Inventory || !SlotWidgetClass) return;

    ClearBar();

    const TArray<FInventorySlot>& Slots = Inventory->GetSlots();
    const int32 End = FMath::Min(StartIndex + HotbarSize, Slots.Num());

    for (int32 i = StartIndex; i < End; ++i)
    {
        UInventorySlotWidget* SlotW = CreateWidget<UInventorySlotWidget>(this, SlotWidgetClass);
        if (!SlotW) continue;

        HotbarBox->AddChildToHorizontalBox(SlotW);
        SlotW->Setup(Inventory.Get(), i, Slots[i].Item, Slots[i].Count);
        SlotW->OnSlotClicked.AddDynamic(this, &UHotbarWidget::OnSlotClicked);
    }
}
