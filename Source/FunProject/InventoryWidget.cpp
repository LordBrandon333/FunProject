#include "InventoryWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/CanvasPanel.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "InventoryComponent.h"
#include "InventorySlotWidget.h"
#include "CursorStackWidget.h"
#include "ItemData.h"
#include "InputCoreTypes.h" // FKey

void UInventoryWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Cursor-Widget erstellen
    if (CursorWidgetClass)
    {
        CursorVisual = CreateWidget<UCursorStackWidget>(GetOwningPlayer(), CursorWidgetClass);
        if (CursorVisual)
        {
            CursorVisual->AddToViewport(1000);
            CursorVisual->SetAlignmentInViewport(FVector2D(0.f, 0.f));
            CursorVisual->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}

void UInventoryWidget::NativeDestruct()
{
    if (Inventory)
    {
        Inventory->OnInventoryChanged.RemoveAll(this);
    }
    if (CursorVisual)
    {
        CursorVisual->RemoveFromParent();
        CursorVisual = nullptr;
    }
    Super::NativeDestruct();
}

void UInventoryWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (CursorVisual && CursorVisual->IsVisible())
    {
        if (APlayerController* PC = GetOwningPlayer())
        {
            const FVector2D MousePos = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetWorld());
            // diese coords sind *nicht* DPI-korrigiert -> false:
            CursorVisual->SetPositionInViewport(MousePos + FVector2D(0.f, 0.f), /*bRemoveDPIScale*/ false);
        }
    }
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
    if (!bShow) CursorClear();
}

void UInventoryWidget::SetCursorEnabled(bool bEnabled)
{
    SetVisible(bEnabled);
    if (!bEnabled) CursorClear();
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

void UInventoryWidget::OnSlotMouseDown(int32 SlotIndex, FKey Button, bool bShift)
{
    if (!Inventory) return;

    const TArray<FInventorySlot>& Slots = Inventory->GetSlots();
    if (!Slots.IsValidIndex(SlotIndex)) return;

    const bool bLeft = (Button == EKeys::LeftMouseButton);
    const bool bRight = (Button == EKeys::RightMouseButton);

    // SHIFT + LMB: Quick-Merge in anderen Stack (nur wenn Cursor leer)
    if (bLeft && bShift && CursorCount == 0)
    {
        UItemData* Item = Slots[SlotIndex].Item;
        if (Item && Slots[SlotIndex].Count > 0)
        {
            const int32 Other = Inventory->FindOtherStackWithSpace(Item, SlotIndex);
            if (Other != INDEX_NONE)
            {
                Inventory->MoveMaxPossible(SlotIndex, Other);
            }
            else
            {
                // << NEU: kein füllbarer Stack -> in andere Region verschieben >>
                const int32 FirstInv = Inventory->GetFirstInventoryIndex();   // == HotbarSize
                const int32 Capacity = Inventory->GetCapacity();
                const int32 HotbarSz = Inventory->GetHotbarSize();

                const bool bClickedInInventory = (SlotIndex >= FirstInv);

                const int32 TargetStart = bClickedInInventory ? 0 : FirstInv;
                const int32 TargetCount = bClickedInInventory ? HotbarSz : (Capacity - FirstInv);

                const int32 EmptyDest = Inventory->FindEmptyIndexInRange(TargetStart, TargetCount);
                if (EmptyDest != INDEX_NONE)
                {
                    Inventory->MoveMaxPossible(SlotIndex, EmptyDest); // verschiebt soviel wie möglich
                }
                // Optional: else -> Feedback anzeigen (kein freier Slot in Zielbereich)
            }
        }
        return;
    }

    // LMB ohne Cursor -> gesamten Stack aufnehmen
    if (bLeft && CursorCount == 0)
    {
        UItemData* Item = nullptr;
        const int32 Taken = Inventory->TakeFromSlot(SlotIndex, INT32_MAX, Item);
        if (Taken > 0 && Item)
        {
            CursorPickup(Item, Taken);
        }
        return;
    }

    // RMB ohne Cursor -> halben Stack aufnehmen (aufgerundet)
    if (bRight && CursorCount == 0)
    {
        const FInventorySlot& S = Slots[SlotIndex];
        if (!S.IsEmpty())
        {
            const int32 Half = FMath::CeilToInt(S.Count * 0.5f);
            UItemData* Item = nullptr;
            const int32 Taken = Inventory->TakeFromSlot(SlotIndex, Half, Item);
            if (Taken > 0 && Item)
            {
                CursorPickup(Item, Taken);
            }
        }
        return;
    }

    // Cursor hat Item:
    if (CursorCount > 0)
    {
        if (bRight)
        {
            // RMB mit Cursor -> EIN Stück platzieren
            CursorPlaceIntoSlot(SlotIndex, /*bSingleUnit=*/true);
        }
        else if (bLeft)
        {
            // LMB mit Cursor -> so viel wie möglich platzieren (merge/fill/swap)
            CursorPlaceIntoSlot(SlotIndex, /*bSingleUnit=*/false);
        }
    }
}

void UInventoryWidget::HandleSlotMouseDown(int32 SlotIndex, FKey Button, bool bShift)
{
    OnSlotMouseDown(SlotIndex, Button, bShift);
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

        if (UUniformGridSlot* GS = ItemsGrid->AddChildToUniformGrid(SlotW, Row, Col))
        {
            GS->SetHorizontalAlignment(HAlign_Fill);
            GS->SetVerticalAlignment(VAlign_Fill);
        }

        SlotW->Setup(Inventory, i, Slots[i].Item, Slots[i].Count);
        SlotW->OnSlotMouseDown.AddDynamic(this, &UInventoryWidget::OnSlotMouseDown);
    }
}

// === Cursor helpers ===
void UInventoryWidget::CursorClear()
{
    CursorItem = nullptr;
    CursorCount = 0;
    if (CursorVisual) CursorVisual->Clear();
}

void UInventoryWidget::CursorPickup(UItemData* Item, int32 Count)
{
    CursorItem = Item;
    CursorCount = Count;
    if (CursorVisual) CursorVisual->SetStack(Item, Count);
}

void UInventoryWidget::CursorPlaceIntoSlot(int32 SlotIndex, bool bSingleUnit)
{
    if (!Inventory || !CursorItem || CursorCount <= 0) return;

    const TArray<FInventorySlot>& Slots = Inventory->GetSlots();
    if (!Slots.IsValidIndex(SlotIndex)) return;

    int32 ToPlace = bSingleUnit ? 1 : CursorCount;
    const int32 Placed = Inventory->TryAddToSlot(SlotIndex, CursorItem, ToPlace);
    CursorCount -= Placed;

    if (Placed == 0 && !bSingleUnit)
    {
        const FInventorySlot& Target = Slots[SlotIndex];
        if (!Target.IsEmpty() && Target.Item != CursorItem)
        {
            // Swap-Logik
            UItemData* OutItem = nullptr;
            const int32 Taken = Inventory->TakeFromSlot(SlotIndex, INT32_MAX, OutItem);

            const int32 PlacedAll = Inventory->TryAddToSlot(SlotIndex, CursorItem, CursorCount);
            if (PlacedAll == CursorCount)
            {
                CursorPickup(OutItem, Taken);
            }
            else
            {
                // Rollback
                if (PlacedAll > 0)
                {
                    UItemData* Dummy = nullptr;
                    Inventory->TakeFromSlot(SlotIndex, PlacedAll, Dummy);
                }
                Inventory->TryAddToSlot(SlotIndex, OutItem, Taken);
            }
        }
    }

    if (CursorCount <= 0)
    {
        CursorClear();
    }
    else if (CursorVisual)
    {
        CursorVisual->SetStack(CursorItem, CursorCount);
    }
}
