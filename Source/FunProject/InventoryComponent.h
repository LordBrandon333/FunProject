#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class UItemData;

USTRUCT(BlueprintType)
struct FInventorySlot
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UItemData> Item = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 Count = 0;

    bool IsEmpty() const { return Item == nullptr || Count <= 0; }
    int32 FreeSpace() const;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHotbarIndexChanged, int32, NewIndex);

UCLASS(ClassGroup = (Survival), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class FUNPROJECT_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

    // === Info
    UFUNCTION(BlueprintPure, Category = "Inventory") int32 GetCapacity() const { return Capacity; }
    UFUNCTION(BlueprintPure, Category = "Inventory") const TArray<FInventorySlot>& GetSlots() const { return Slots; }
    UFUNCTION(BlueprintPure, Category = "Inventory") int32 GetHotbarSize() const { return HotbarSize; }
    UFUNCTION(BlueprintPure, Category = "Inventory") int32 GetFirstInventoryIndex() const { return FMath::Clamp(HotbarSize, 0, Capacity); }

    // --- Mutations
    UFUNCTION(BlueprintCallable, Category = "Inventory") int32 TryAddItem(UItemData* Item, int32 Count);
    UFUNCTION(BlueprintCallable, Category = "Inventory") int32 RemoveItem(UItemData* Item, int32 Count);
    UFUNCTION(BlueprintPure, Category = "Inventory") int32 GetCountOf(UItemData* Item) const;
    UFUNCTION(BlueprintCallable, Category = "Inventory") bool  UseSlot(int32 SlotIndex);

    // --- Select / Hotbar
    UFUNCTION(BlueprintPure, Category = "Inventory|Hotbar") int32 GetSelectedHotbarIndex() const { return SelectedHotbarIndex; }
    UFUNCTION(BlueprintCallable, Category = "Inventory|Hotbar") bool  SetSelectedHotbarIndex(int32 NewIndex);
    UFUNCTION(BlueprintCallable, Category = "Inventory|Hotbar") void  OffsetSelectedHotbarIndex(int32 Delta);
    UFUNCTION(BlueprintCallable, Category = "Inventory|Hotbar") bool  UseSelectedHotbarItem();

    // --- Slot-specific Operations
    UFUNCTION(BlueprintCallable, Category = "Inventory|Slots") int32 TryAddToSlot(int32 SlotIndex, UItemData* Item, int32 Count);
    UFUNCTION(BlueprintCallable, Category = "Inventory|Slots") int32 TakeFromSlot(int32 SlotIndex, int32 Count, UItemData*& OutItem);
    UFUNCTION(BlueprintCallable, Category = "Inventory|Slots") int32 MoveMaxPossible(int32 FromIndex, int32 ToIndex);
    UFUNCTION(BlueprintCallable, Category = "Inventory|Slots") bool SwapSlots(int32 AIndex, int32 BIndex);
    UFUNCTION(BlueprintPure, Category = "Inventory|Slots") int32 FindOtherStackWithSpace(UItemData* Item, int32 ExcludeIndex) const;

    // --- Events
    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events") FOnInventoryChanged    OnInventoryChanged;
    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events") FOnHotbarIndexChanged  OnHotbarIndexChanged;

protected:
    virtual void OnRegister() override;
    virtual void InitializeComponent() override;
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "1"))
    int32 Capacity = 28; 

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "0"))
    int32 HotbarSize = 4;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TArray<FInventorySlot> Slots;

private:
    int32 FindFirstStackableIndex(UItemData* Item) const;
    int32 FindEmptyIndex() const;
    void  BroadcastChanged();
    void RefreshSizing();

    UPROPERTY(VisibleAnywhere, Category = "Inventory|Hotbar")
    int32 SelectedHotbarIndex = 0;
};
