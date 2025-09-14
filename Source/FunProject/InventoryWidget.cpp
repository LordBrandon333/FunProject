#include "InventoryWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/PanelSlot.h"
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

    ItemsGrid->SetSlotPadding(FMargin(4.f)); 
    ItemsGrid->SetMinDesiredSlotWidth(64.f);     
    ItemsGrid->SetMinDesiredSlotHeight(64.f);

    const TArray<FInventorySlot>& Slots = Inventory->GetSlots();
    const int32 End = FMath::Clamp(StartIndex + VisibleCount, 0, Slots.Num());

    for (int32 i = StartIndex; i < End; ++i)
    {
        UInventorySlotWidget* SlotW = CreateWidget<UInventorySlotWidget>(this, SlotWidgetClass);
        if (!SlotW) continue;

        const int32 Visual = i - StartIndex;
        const int32 Row = Visual / Columns;
        const int32 Col = Visual % Columns;

        if (UUniformGridSlot* GS = Cast<UUniformGridSlot>(ItemsGrid->AddChildToUniformGrid(SlotW, Row, Col)))
        {
            GS->SetHorizontalAlignment(HAlign_Fill);
            GS->SetVerticalAlignment(VAlign_Fill);
        }

        SlotW->Setup(Inventory.Get(), i, Slots[i].Item, Slots[i].Count);
        SlotW->OnSlotClicked.AddDynamic(this, &UInventoryWidget::OnSlotClicked);
    }
}
