#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemData.generated.h"

class UTexture2D;
class UItemAction;

UENUM(BlueprintType)
enum class EItemType : uint8
{
    Consumable UMETA(DisplayName = "Consumable"),
    Material   UMETA(DisplayName = "Material"),
    Tool       UMETA(DisplayName = "Tool"),
    KeyItem    UMETA(DisplayName = "Key Item"),
};

/** Ein Item ist datengetrieben + Liste von Actions (Komposition). */
UCLASS(BlueprintType)
class FUNPROJECT_API UItemData : public UDataAsset
{
    GENERATED_BODY()
public:
    // Identität/Anzeige
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FName ItemId = NAME_None;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    EItemType Type = EItemType::Consumable;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    TObjectPtr<UTexture2D> Icon = nullptr;

    // Stacking/Weight
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stacking", meta = (ClampMin = "1"))
    int32 MaxStackSize = 20;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weight", meta = (ClampMin = "0"))
    float WeightPerUnit = 0.f;

    // Verhalten: frei kombinierbare Actions (z. B. Heal, NeedDelta, ApplyEffect, ...)
    UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly, Category = "OnUse")
    TArray<TObjectPtr<UItemAction>> ActionsOnUse;

    // Wird beim Benutzen eine Einheit verbraucht?
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OnUse")
    bool bConsumeOnUse = true;
};
