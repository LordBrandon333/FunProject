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
    int32 FreeSpace() const; // per Item->MaxStackSize
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

UCLASS(ClassGroup = (Survival), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class FUNPROJECT_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetCapacity() const { return Capacity; }

    UFUNCTION(BlueprintPure, Category = "Inventory")
    const TArray<FInventorySlot>& GetSlots() const { return Slots; }

    // Versucht Count eines Items hinzuzufügen; gibt tatsächlich hinzugefügte Menge zurück
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 TryAddItem(UItemData* Item, int32 Count);

    // Entfernt bis zu Count des Items; gibt tatsächlich entfernte Menge zurück
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 RemoveItem(UItemData* Item, int32 Count);

    // Summe im Inventar
    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetCountOf(UItemData* Item) const;

    // Benutze Item im SlotIndex (führt Actions aus)
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool UseSlot(int32 SlotIndex);

    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnInventoryChanged OnInventoryChanged;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "1"))
    int32 Capacity = 24;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TArray<FInventorySlot> Slots;

private:
    int32 FindFirstStackableIndex(UItemData* Item) const;
    int32 FindEmptyIndex() const;
    void BroadcastChanged();

    bool ApplyUseEffects(UItemData* Item); // iteriert ActionsOnUse und führt sie aus
};
