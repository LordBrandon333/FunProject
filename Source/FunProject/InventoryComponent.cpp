#include "InventoryComponent.h"
#include "ItemData.h"
#include "ItemAction.h"

int32 FInventorySlot::FreeSpace() const
{
    if (!Item) return 0;
    return FMath::Max(0, Item->MaxStackSize - Count);
}

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    Slots.SetNum(Capacity);
}

int32 UInventoryComponent::FindFirstStackableIndex(UItemData* Item) const
{
    if (!Item) return INDEX_NONE;
    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        const FInventorySlot& S = Slots[i];
        if (S.Item == Item && S.Count < Item->MaxStackSize)
            return i;
    }
    return INDEX_NONE;
}

int32 UInventoryComponent::FindEmptyIndex() const
{
    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        if (Slots[i].IsEmpty()) return i;
    }
    return INDEX_NONE;
}

int32 UInventoryComponent::TryAddItem(UItemData* Item, int32 Count)
{
    if (!Item || Count <= 0) return 0;

    int32 ToAdd = Count;

    // 1) In bestehende Stacks füllen
    while (ToAdd > 0)
    {
        const int32 StackIdx = FindFirstStackableIndex(Item);
        if (StackIdx == INDEX_NONE) break;

        FInventorySlot& S = Slots[StackIdx];
        const int32 Space = S.FreeSpace();
        const int32 Move = FMath::Min(Space, ToAdd);
        S.Count += Move;
        ToAdd -= Move;
    }

    // 2) Neue Stacks in leere Slots
    while (ToAdd > 0)
    {
        const int32 EmptyIdx = FindEmptyIndex();
        if (EmptyIdx == INDEX_NONE) break;

        FInventorySlot& S = Slots[EmptyIdx];
        S.Item = Item;
        const int32 Move = FMath::Min(Item->MaxStackSize, ToAdd);
        S.Count = Move;
        ToAdd -= Move;
    }

    const int32 Added = Count - ToAdd;
    if (Added > 0) BroadcastChanged();
    return Added;
}

int32 UInventoryComponent::RemoveItem(UItemData* Item, int32 Count)
{
    if (!Item || Count <= 0) return 0;

    int32 ToRemove = Count;

    for (int32 i = 0; i < Slots.Num() && ToRemove > 0; ++i)
    {
        FInventorySlot& S = Slots[i];
        if (S.Item != Item) continue;

        const int32 Take = FMath::Min(S.Count, ToRemove);
        S.Count -= Take;
        ToRemove -= Take;

        if (S.Count <= 0) { S.Item = nullptr; S.Count = 0; }
    }

    const int32 Removed = Count - ToRemove;
    if (Removed > 0) BroadcastChanged();
    return Removed;
}

int32 UInventoryComponent::GetCountOf(UItemData* Item) const
{
    if (!Item) return 0;
    int32 Sum = 0;
    for (const FInventorySlot& S : Slots)
    {
        if (S.Item == Item) Sum += S.Count;
    }
    return Sum;
}

bool UInventoryComponent::UseSlot(int32 SlotIndex)
{
    if (!Slots.IsValidIndex(SlotIndex)) return false;

    FInventorySlot& S = Slots[SlotIndex];
    if (S.IsEmpty() || !S.Item) return false;

    // Execute Actions (kein harter Check auf EItemType)
    const bool bDid = ApplyUseEffects(S.Item);
    if (!bDid) return false;

    if (S.Item->bConsumeOnUse)
    {
        S.Count -= 1;
        if (S.Count <= 0) { S.Item = nullptr; S.Count = 0; }
    }

    BroadcastChanged();
    return true;
}

bool UInventoryComponent::ApplyUseEffects(UItemData* Item)
{
    if (!Item) return false;
    AActor* Owner = GetOwner();
    if (!Owner) return false;

    bool bAny = false;
    for (UItemAction* Action : Item->ActionsOnUse)
    {
        if (Action && Action->Execute(Owner)) bAny = true;
    }
    return bAny;
}

void UInventoryComponent::BroadcastChanged()
{
    OnInventoryChanged.Broadcast();
}
