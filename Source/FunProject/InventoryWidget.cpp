#include "InventoryWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "InventoryComponent.h"
#include "InventorySlotWidget.h"
#include "ItemData.h"

void UInventoryWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UInventoryWidget::NativeDestruct()
{
    if (Inventory)
    {
        Inventory->OnInventoryChanged.RemoveAll(this);
    }
    Super::NativeDestruct();
}

void UInventoryWidget::InitializeForInventory(UInventoryComponent* InInventory)
{
    if (!InInventory) return;

    if (Inventory)
    {
        Inventory->OnInventoryChanged.RemoveAll(this);
    }

    Inventory = InInventory;
    Inventory->OnInventoryChanged.AddDynamic(this, &UInventoryWidget::OnInventoryChanged);

    RebuildGrid();
}

void UInventoryWidget::SetVisible(bool bShow)
{
    SetVisibility(bShow ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}

void UInventoryWidget::OnInventoryChanged()
{
    RebuildGrid();
}

void UInventoryWidget::OnSlotClicked(int32 SlotIndex)
{
    if (!Inventory) return;
    Inventory->UseSlot(SlotIndex);
}

void UInventoryWidget::ClearGrid()
{
    if (ItemsGrid)
    {
        ItemsGrid->ClearChildren();
    }
}

void UInventoryWidget::RebuildGrid()
{
    if (!ItemsGrid || !Inventory || !SlotWidgetClass) return;

    ClearGrid();

    const TArray<FInventorySlot>& Slots = Inventory->GetSlots();
    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        UInventorySlotWidget* SlotW = CreateWidget<UInventorySlotWidget>(this, SlotWidgetClass);
        if (!SlotW) continue;

        const int32 Row = i / Columns;
        const int32 Col = i % Columns;

        ItemsGrid->AddChildToUniformGrid(SlotW, Row, Col);

        SlotW->Setup(Inventory.Get(), i, Slots[i].Item, Slots[i].Count);
        SlotW->OnSlotClicked.AddDynamic(this, &UInventoryWidget::OnSlotClicked);
    }
}
