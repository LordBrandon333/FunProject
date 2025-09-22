#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "Engine/StaticMesh.h"
#include "GameplayTagContainer.h"
#include "ItemDataStructs.generated.h"

class UItemAction;
class UItemFragment;

UENUM(BlueprintType)
enum class EItemQuality : uint8
{
    Common      UMETA(DisplayName = "Common"),
    Rare        UMETA(DisplayName = "Rare"),
    Quality     UMETA(DisplayName = "Quality"),
    Masterful   UMETA(DisplayName = "Masterful"),
    Legendary   UMETA(DisplayName = "Legendary"),
};

UENUM(BlueprintType)
enum class EItemTypeClass : uint8
{
    Consumable UMETA(DisplayName = "Consumable"),
    Material   UMETA(DisplayName = "Material"),
    Tool       UMETA(DisplayName = "Tool"),
    Weapon     UMETA(DisplayName = "Weapon"),
    Clothing   UMETA(DisplayName = "Clothing"),
    Accessoire UMETA(DisplayName = "Accessoire"),
    KeyItem    UMETA(DisplayName = "Key Item"),
    Mundane    UMETA(DisplayName = "Mundane"),
};

USTRUCT(BlueprintType)
struct FItemTextData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Description")
    FText Name;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Description")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Description")
    FText InteractionText;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Description")
    FText UsageText;
};

USTRUCT(BlueprintType)
struct FItemNumericData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Numerics", meta = (ClampMin = "0.0"))
    float Weight = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Numerics", meta = (ClampMin = "1"))
    int32 MaxStackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Numerics")
    bool bIsStackable = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Numerics", meta = (ClampMin = "0"))
    int32 BaseValue = 0;
};

USTRUCT(BlueprintType)
struct FItemAssetData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Assets")
    TObjectPtr<UTexture2D> Icon = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Assets")
    TObjectPtr<UStaticMesh> Mesh = nullptr;
};

UENUM(BlueprintType)
enum class EItemActionSlot : uint8
{
    Primary,
    Secondary,
    Tertiary
};


USTRUCT(BlueprintType)
struct FActionBinding
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Actions")
    TSoftClassPtr<UItemAction> ActionClass;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Actions")
    EItemActionSlot Slot = EItemActionSlot::Primary;
};


USTRUCT(BlueprintType)
struct FItemDataRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
    FName ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
    EItemTypeClass ItemType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
    EItemQuality ItemQuality;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
    FItemTextData TextData;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
    FItemNumericData NumericData;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
    FItemAssetData AssetData;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
    FGameplayTagContainer Tags; 
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
    TArray<TSoftClassPtr<UItemFragment>> FragmentClasses;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
    TArray<FActionBinding> DefaultActions;
};
