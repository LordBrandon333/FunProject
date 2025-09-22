#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemDataStructs.generated.h"

class UTexture2D;
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
    float Weight;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Numerics", meta = (ClampMin = "1"))
    int32 MaxStackSize;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Numerics")
    bool bIsStackable;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Numerics", meta = (ClampMin = "0"))
    int32 BaseValue;
};

USTRUCT(BlueprintType)
struct FItemAssetData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Assets")
    UTexture2D* Icon;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Assets")
    UStaticMesh* Mesh;
};


USTRUCT(BlueprintType)
struct FItemDataClas
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
    EItemType ItemType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
    EItemQuality ItemQuality;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
    FItemTextData TextData;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
    FItemNumericData NumericData;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
    FItemAssetData AssetData;
};
