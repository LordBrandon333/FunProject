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

void UInventoryComponent::InitializeComponent()
{
    Super::InitializeComponent();

    Slots.SetNum(Capacity);

    if (HotbarSize > Capacity)
    {
        UE_LOG(LogTemp, Warning, TEXT("InventoryComponent: HotbarSize (%d) > Capacity (%d). Clamping."),
            HotbarSize, Capacity);
        HotbarSize = Capacity;
    }

    SelectedHotbarIndex = FMath::Clamp(SelectedHotbarIndex, 0, FMath::Max(HotbarSize - 1, 0));

    RefreshSizing();
    OnInventoryChanged.Broadcast();
    OnHotbarIndexChanged.Broadcast(SelectedHotbarIndex);
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    OnInventoryChanged.Broadcast();
    OnHotbarIndexChanged.Broadcast(SelectedHotbarIndex);
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

    // Aktionen ausführen (ItemData → ActionsOnUse)
    bool bAny = false;
    for (UItemAction* Action : S.Item->ActionsOnUse)
    {
        if (Action && Action->Execute(GetOwner())) bAny = true;
    }
    if (!bAny) return false;

    if (S.Item->bConsumeOnUse)
    {
        S.Count -= 1;
        if (S.Count <= 0) { S.Item = nullptr; S.Count = 0; }
    }

    BroadcastChanged();
    return true;
}

bool UInventoryComponent::SetSelectedHotbarIndex(int32 NewIndex)
{
    if (HotbarSize <= 0) return false;
    NewIndex = FMath::Clamp(NewIndex, 0, HotbarSize - 1);
    if (NewIndex == SelectedHotbarIndex) return false;
    SelectedHotbarIndex = NewIndex;
    OnHotbarIndexChanged.Broadcast(SelectedHotbarIndex);
    return true;
}

void UInventoryComponent::OffsetSelectedHotbarIndex(int32 Delta)
{
    if (HotbarSize <= 0) return;
    const int32 H = HotbarSize;
    int32 NewIndex = (SelectedHotbarIndex + Delta) % H;
    if (NewIndex < 0) NewIndex += H;
    if (NewIndex != SelectedHotbarIndex)
    {
        SelectedHotbarIndex = NewIndex;
        OnHotbarIndexChanged.Broadcast(SelectedHotbarIndex);
    }
}

bool UInventoryComponent::UseSelectedHotbarItem()
{
    if (HotbarSize <= 0) return false;
    return UseSlot(SelectedHotbarIndex);
}

void UInventoryComponent::BroadcastChanged()
{
    OnInventoryChanged.Broadcast();
}

void UInventoryComponent::RefreshSizing()
{
    // Capacity clampen
    if (Capacity < 0) Capacity = 0;

    // Slots auf Capacity bringen
    if (Slots.Num() != Capacity)
        Slots.SetNum(Capacity);

    // Hotbar clampen
    if (HotbarSize > Capacity)
        HotbarSize = Capacity;

    SelectedHotbarIndex = FMath::Clamp(SelectedHotbarIndex, 0, FMath::Max(HotbarSize - 1, 0));
}

void UInventoryComponent::OnRegister()
{
    Super::OnRegister();
    RefreshSizing();
}
