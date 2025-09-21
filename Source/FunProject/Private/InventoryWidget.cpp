#include "InventoryWidget.h"

#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/CanvasPanel.h"
#include "Blueprint/WidgetLayoutLibrary.h"

#include "InventoryComponent.h"
#include "InventorySlotWidget.h"
#include "CursorStackWidget.h"
#include "SplitStackWidget.h"
#include "ItemData.h"

#include "InputCoreTypes.h"
#include "Framework/Application/SlateApplication.h" // für Alt-Modifier

void UInventoryWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Cursor-Widget erzeugen
    if (CursorWidgetClass)
    {
        CursorVisual = CreateWidget<UCursorStackWidget>(GetOwningPlayer(), CursorWidgetClass);
        if (CursorVisual)
        {
            CursorVisual->AddToViewport(1000);
            CursorVisual->SetAlignmentInViewport(FVector2D(0.f, 0.f)); // kein Pivot-Offset
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
    ActiveSplit = nullptr;

    Super::NativeDestruct();
}

void UInventoryWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (CursorVisual && CursorVisual->IsVisible())
    {
        // Koordinaten ohne DPI-Skalierung -> bRemoveDPIScale=false
        const FVector2D MousePos = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetWorld());
        CursorVisual->SetPositionInViewport(MousePos + FVector2D(12.f, 12.f), /*bRemoveDPIScale*/ false);
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

void UInventoryWidget::HandleSlotMouseDown(int32 SlotIndex, FKey Button, bool bShift)
{
    OnSlotMouseDown(SlotIndex, Button, bShift);
}

void UInventoryWidget::OnSlotMouseDown(int32 SlotIndex, FKey Button, bool bShift)
{
    if (!Inventory) return;

    const TArray<FInventorySlot>& Slots = Inventory->GetSlots();
    if (!Slots.IsValidIndex(SlotIndex)) return;

    const bool bLeft = (Button == EKeys::LeftMouseButton);
    const bool bRight = (Button == EKeys::RightMouseButton);
    const bool bAlt = FSlateApplication::Get().GetModifierKeys().IsAltDown();

    // === ALT + LMB: Split-Dialog (nur wenn Cursor leer) ===
    if (bLeft && bAlt && CursorCount == 0)
    {
        const FInventorySlot& S = Slots[SlotIndex];
        if (!S.IsEmpty() && S.Count > 1)
        {
            OpenSplitDialog(SlotIndex, S.Count - 1);
        }
        return;
    }

    // === SHIFT + LMB: Ganzen Stack Bereich A -> Bereich B schieben ===
    if (bLeft && bShift && CursorCount == 0)
    {
        UItemData* Item = Slots[SlotIndex].Item;
        if (!Item || Slots[SlotIndex].Count <= 0) return;

        const int32 FirstInv = Inventory->GetFirstInventoryIndex(); // == HotbarSize
        const int32 Capacity = Inventory->GetCapacity();
        const int32 HotbarSz = Inventory->GetHotbarSize();
        const bool  bFromInv = (SlotIndex >= FirstInv);

        const int32 TargetStart = bFromInv ? 0 : FirstInv;
        const int32 TargetCount = bFromInv ? HotbarSz : (Capacity - FirstInv);

        int safety = 0;
        while (safety++ < Capacity)
        {
            const TArray<FInventorySlot>& Cur = Inventory->GetSlots();
            if (Cur[SlotIndex].IsEmpty())
                break;

            // 1) Teilgefüllten Stack im Zielbereich füllen
            int32 dest = FindPartialStackInRange(Item, INDEX_NONE, TargetStart, TargetCount);
            if (dest != INDEX_NONE)
            {
                Inventory->MoveMaxPossible(SlotIndex, dest);
                continue;
            }

            // 2) Freien Slot im Zielbereich nutzen
            const int32 emptyDest = Inventory->FindEmptyIndexInRange(TargetStart, TargetCount);
            if (emptyDest != INDEX_NONE)
            {
                Inventory->MoveMaxPossible(SlotIndex, emptyDest);
                continue;
            }

            // 3) Zielbereich voll -> Ende
            break;
        }
        return;
    }

    // === RMB: ohne Cursor -> halben Stack aufnehmen ===
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

    // === RMB: mit Cursor -> 1 Stück platzieren ===
    if (bRight && CursorCount > 0)
    {
        CursorPlaceIntoSlot(SlotIndex, /*bSingleUnit=*/true);
        return;
    }

    // === LMB: ohne Cursor -> ganzen Stack aufnehmen ===
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

    // === LMB: mit Cursor -> soviel wie möglich platzieren (merge/fill/swap) ===
    if (bLeft && CursorCount > 0)
    {
        CursorPlaceIntoSlot(SlotIndex, /*bSingleUnit=*/false);
        return;
    }
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

// === Split helpers ===
int32 UInventoryWidget::FindPartialStackInRange(UItemData* Item, int32 /*ExcludeIndex*/, int32 Start, int32 Count) const
{
    if (!Inventory || !Item) return INDEX_NONE;
    const TArray<FInventorySlot>& SlotsRef = Inventory->GetSlots();
    const int32 S = FMath::Clamp(Start, 0, SlotsRef.Num());
    const int32 E = FMath::Clamp(Start + Count, 0, SlotsRef.Num());

    for (int32 i = S; i < E; ++i)
    {
        const FInventorySlot& Slt = SlotsRef[i];
        if (Slt.Item == Item && Slt.Count > 0 && Slt.Count < Item->MaxStackSize)
            return i;
    }
    return INDEX_NONE;
}

void UInventoryWidget::OpenSplitDialog(int32 SlotIndex, int32 MaxCount)
{
    if (!SplitWidgetClass) return;
    if (ActiveSplit) { ActiveSplit->RemoveFromParent(); ActiveSplit = nullptr; }

    ActiveSplit = CreateWidget<USplitStackWidget>(GetOwningPlayer(), SplitWidgetClass);
    if (ActiveSplit)
    {
        ActiveSplit->AddToViewport(999);
        ActiveSplit->SetIsFocusable(true);
        ActiveSplit->Init(this, SlotIndex, MaxCount);

        if (APlayerController* PC = GetOwningPlayer())
        {
            FInputModeGameAndUI M;
            M.SetWidgetToFocus(ActiveSplit->TakeWidget());
            M.SetHideCursorDuringCapture(false);
            PC->SetInputMode(M);
            PC->bShowMouseCursor = true;
        }
    }
}

void UInventoryWidget::ConfirmSplit(int32 SlotIndex, int32 Amount)
{
    if (!Inventory || Amount <= 0) { ActiveSplit = nullptr; return; }

    UItemData* Item = nullptr;
    const int32 Taken = Inventory->TakeFromSlot(SlotIndex, Amount, Item);
    if (Taken > 0 && Item)
    {
        CursorPickup(Item, Taken); // Cursor trägt jetzt Teil-Stack
    }
    ActiveSplit = nullptr;
}

void UInventoryWidget::CancelSplit()
{
    ActiveSplit = nullptr;
}
