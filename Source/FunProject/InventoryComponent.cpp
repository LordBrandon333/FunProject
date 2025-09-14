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

int32 UInventoryComponent::TryAddToSlot(int32 SlotIndex, UItemData* Item, int32 Count)
{
    if (!Slots.IsValidIndex(SlotIndex) || !Item || Count <= 0) return 0;

    FInventorySlot& S = Slots[SlotIndex];

    if (S.IsEmpty())
    {
        const int32 Move = FMath::Min(Item->MaxStackSize, Count);
        S.Item = Item;
        S.Count = Move;
        if (Move > 0) BroadcastChanged();
        return Move;
    }

    if (S.Item != Item) return 0;

    const int32 Space = FMath::Max(0, Item->MaxStackSize - S.Count);
    const int32 Move = FMath::Min(Space, Count);
    S.Count += Move;
    if (Move > 0) BroadcastChanged();
    return Move;
}

int32 UInventoryComponent::TakeFromSlot(int32 SlotIndex, int32 Count, UItemData*& OutItem)
{
    if (!Slots.IsValidIndex(SlotIndex) || Count <= 0) { OutItem = nullptr; return 0; }

    FInventorySlot& S = Slots[SlotIndex];
    if (S.IsEmpty()) { OutItem = nullptr; return 0; }

    OutItem = S.Item;
    const int32 Take = FMath::Min(S.Count, Count);
    S.Count -= Take;
    if (S.Count <= 0) { S.Item = nullptr; S.Count = 0; }
    if (Take > 0) BroadcastChanged();
    return Take;
}

int32 UInventoryComponent::MoveMaxPossible(int32 FromIndex, int32 ToIndex)
{
    if (!Slots.IsValidIndex(FromIndex) || !Slots.IsValidIndex(ToIndex) || FromIndex == ToIndex) return 0;

    FInventorySlot& A = Slots[FromIndex];
    FInventorySlot& B = Slots[ToIndex];
    if (A.IsEmpty()) return 0;

    if (B.IsEmpty())
    {
        const int32 Move = FMath::Min(A.Count, A.Item->MaxStackSize);
        B.Item = A.Item;
        B.Count = Move;
        A.Count -= Move;
        if (A.Count <= 0) { A.Item = nullptr; A.Count = 0; }
        if (Move > 0) BroadcastChanged();
        return Move;
    }

    if (B.Item != A.Item) return 0;

    const int32 Space = FMath::Max(0, B.Item->MaxStackSize - B.Count);
    const int32 Move = FMath::Min(Space, A.Count);
    B.Count += Move;
    A.Count -= Move;
    if (A.Count <= 0) { A.Item = nullptr; A.Count = 0; }
    if (Move > 0) BroadcastChanged();
    return Move;
}

bool UInventoryComponent::SwapSlots(int32 AIndex, int32 BIndex)
{
    if (!Slots.IsValidIndex(AIndex) || !Slots.IsValidIndex(BIndex) || AIndex == BIndex) return false;
    Slots.Swap(AIndex, BIndex);
    BroadcastChanged();
    return true;
}

int32 UInventoryComponent::FindOtherStackWithSpace(UItemData* Item, int32 ExcludeIndex) const
{
    if (!Item) return INDEX_NONE;
    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        if (i == ExcludeIndex) continue;
        const FInventorySlot& S = Slots[i];
        if (S.Item == Item && S.Count < Item->MaxStackSize) return i;
    }
    return INDEX_NONE;
}

int32 UInventoryComponent::FindEmptyIndexInRange(int32 StartIndex, int32 Count) const
{
    if (Slots.Num() == 0 || Count <= 0) return INDEX_NONE;
    const int32 Start = FMath::Clamp(StartIndex, 0, Slots.Num());
    const int32 End = FMath::Clamp(Start + Count, 0, Slots.Num());
    for (int32 i = Start; i < End; ++i)
    {
        if (Slots[i].IsEmpty()) return i;
    }
    return INDEX_NONE;
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
