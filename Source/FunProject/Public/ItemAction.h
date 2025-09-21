#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ItemAction.generated.h"

/**
 * Basisklasse für Item-Aktionen. Wird beim Benutzen des Items ausgeführt.
 * Rückgabe: true, wenn die Aktion etwas getan hat (für SFX/Feedback hilfreich).
 */
UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class FUNPROJECT_API UItemAction : public UObject
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintNativeEvent, Category = "ItemAction")
    bool Execute(AActor* OwnerActor);
    virtual bool Execute_Implementation(AActor* OwnerActor);
};

/** Heilt den Owner um Amount. */
UCLASS(BlueprintType, EditInlineNew)
class FUNPROJECT_API UItemAction_Heal : public UItemAction
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heal", meta = (ClampMin = "0"))
    float Amount = 0.f;

    virtual bool Execute_Implementation(AActor* OwnerActor) override;
};

/** Ändert einen Need (Hunger/Thirst etc.) um Amount (positiv = füllen, negativ = konsumieren). */
UCLASS(BlueprintType, EditInlineNew)
class FUNPROJECT_API UItemAction_NeedDelta : public UItemAction
{
    GENERATED_BODY()
public:
    /** Ziels-Komponentenklasse, z. B. UHungerComponent oder UThirstComponent */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Need")
    TSubclassOf<class UNeedComponent> TargetNeed;

    /** Positiv -> Add(); Negativ -> Consume(-Amount) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Need")
    float Amount = 0.f;

    virtual bool Execute_Implementation(AActor* OwnerActor) override;
};

/** Wendet einen StatusEffect (DataAsset) an. */
UCLASS(BlueprintType, EditInlineNew)
class FUNPROJECT_API UItemAction_ApplyEffect : public UItemAction
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
    class UStatusEffectData* Effect = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
    FName SourceId = TEXT("ItemUse");

    virtual bool Execute_Implementation(AActor* OwnerActor) override;
};
