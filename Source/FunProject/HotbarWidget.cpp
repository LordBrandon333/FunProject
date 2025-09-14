#include "HotbarWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "InventoryComponent.h"
#include "InventorySlotWidget.h"
#include "ItemData.h"
#include "InventoryWidget.h"
#include "InputCoreTypes.h"

void UHotbarWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UHotbarWidget::NativeDestruct()
{
    if (Inventory)
    {
        Inventory->OnInventoryChanged.RemoveAll(this);
        Inventory->OnHotbarIndexChanged.RemoveAll(this);
    }
    Super::NativeDestruct();
}

void UHotbarWidget::InitializeForInventory(UInventoryComponent* InInventory)
{
    if (Inventory)
    {
        Inventory->OnInventoryChanged.RemoveAll(this);
        Inventory->OnHotbarIndexChanged.RemoveAll(this);
    }

    Inventory = InInventory;

    if (Inventory)
    {
        Inventory->OnInventoryChanged.AddDynamic(this, &UHotbarWidget::OnInventoryChanged);
        Inventory->OnHotbarIndexChanged.AddDynamic(this, &UHotbarWidget::OnHotbarIndexChanged);

        if (HotbarSize <= 0) HotbarSize = Inventory->GetHotbarSize();
        StartIndex = 0;
    }

    RebuildBar();
    UpdateSelection();
}

void UHotbarWidget::SetHotbarRange(int32 InStartIndex, int32 InSize)
{
    StartIndex = FMath::Max(0, InStartIndex);
    HotbarSize = FMath::Max(1, InSize);
    RebuildBar();
    UpdateSelection();
}

void UHotbarWidget::OnInventoryChanged()
{
    RebuildBar();
    UpdateSelection();
}

void UHotbarWidget::OnHotbarIndexChanged(int32 /*NewIndex*/)
{
    UpdateSelection();
}

void UHotbarWidget::OnSlotClicked(int32 SlotIndex)
{
    if (Inventory)
    {
        if (SlotIndex >= StartIndex && SlotIndex < StartIndex + HotbarSize)
        {
            Inventory->SetSelectedHotbarIndex(SlotIndex - StartIndex);
        }
    }
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
        SlotW->Setup(Inventory, i, Slots[i].Item, Slots[i].Count);
        SlotW->OnSlotClicked.AddDynamic(this, &UHotbarWidget::OnSlotClicked);
        SlotW->OnSlotMouseDown.AddDynamic(this, &UHotbarWidget::OnSlotMouseDownFromHotbar);
    }
}

void UHotbarWidget::UpdateSelection()
{
    if (!Inventory || !HotbarBox) return;

    const int32 Selected = Inventory->GetSelectedHotbarIndex();
    const int32 ChildCount = HotbarBox->GetChildrenCount();

    for (int32 c = 0; c < ChildCount; ++c)
    {
        if (auto* SlotW = Cast<UInventorySlotWidget>(HotbarBox->GetChildAt(c)))
        {
            const int32 AbsoluteIndex = StartIndex + c;
            const bool bIsSelected = (AbsoluteIndex == (StartIndex + Selected));
            SlotW->SetSelected(bIsSelected);
        }
    }
}

void UHotbarWidget::OnSlotMouseDownFromHotbar(int32 SlotIndex, FKey Button, bool bShift)
{
    // Wenn Inventar offen ist: dieselbe Cursor-Logik wie im InventoryWidget nutzen
    if (InventoryUI &&
        (InventoryUI->GetVisibility() == ESlateVisibility::Visible ||
            InventoryUI->GetVisibility() == ESlateVisibility::SelfHitTestInvisible))
    {
        InventoryUI->HandleSlotMouseDown(SlotIndex, Button, bShift);
        return;
    }

    // Wenn Inventar zu ist: Klick soll nur Hotbar-Slot AUSWÄHLEN (nicht verschieben)
    if (Inventory)
    {
        if (SlotIndex >= StartIndex && SlotIndex < StartIndex + HotbarSize)
        {
            Inventory->SetSelectedHotbarIndex(SlotIndex - StartIndex);
        }
    }
}
