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
    if (Inventory)
    {
        Inventory->OnInventoryChanged.RemoveAll(this);
    }

    Inventory = InInventory;
    if (Inventory)
    {
        Inventory->OnInventoryChanged.AddDynamic(this, &UInventoryWidget::OnInventoryChanged);

        if (VisibleCount <= 0)
        {
            const int32 FirstInv = Inventory->GetFirstInventoryIndex();
            StartIndex = FirstInv;
            VisibleCount = Inventory->GetCapacity() - FirstInv;
        }
    }

    RebuildGrid();
}

void UInventoryWidget::SetVisible(bool bShow)
{
    SetVisibility(bShow ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}

void UInventoryWidget::SetRange(int32 InStartIndex, int32 InCount)
{
    StartIndex = FMath::Max(0, InStartIndex);
    VisibleCount = FMath::Max(0, InCount);
    RebuildGrid();
}

void UInventoryWidget::OnInventoryChanged()
{
    RebuildGrid();
}

void UInventoryWidget::OnSlotClicked(int32 /*SlotIndex*/)
{
    // (für später: Drag/Drop/Split)
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
    const int32 End = FMath::Clamp(StartIndex + VisibleCount, 0, Slots.Num());

    ItemsGrid->SetSlotPadding(FMargin(4.f));
    ItemsGrid->SetMinDesiredSlotWidth(64.f);
    ItemsGrid->SetMinDesiredSlotHeight(64.f);

    for (int32 i = StartIndex; i < End; ++i)
    {
        UInventorySlotWidget* SlotW = CreateWidget<UInventorySlotWidget>(this, SlotWidgetClass);
        if (!SlotW) continue;

        const int32 visual = i - StartIndex;
        const int32 row = visual / Columns;
        const int32 col = visual % Columns;

        ItemsGrid->AddChildToUniformGrid(SlotW, row, col);
        SlotW->Setup(Inventory, i, Slots[i].Item, Slots[i].Count);
    }
}
