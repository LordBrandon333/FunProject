#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h" // dein Interface
#include "ItemPickup.generated.h"

class UStaticMeshComponent;
class UItemData;
class UInventoryComponent;
class APlayerCharacter;

UCLASS()
class FUNPROJECT_API AItemPickup : public AActor, public IInteractable
{
    GENERATED_BODY()
public:
    AItemPickup();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    TObjectPtr<UItemData> Item = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "1"))
    int32 Count = 1;

public:
    // IInteractable
    virtual bool CanInteract_Implementation(APlayerCharacter* Interactor) override;
    virtual void Interact_Implementation(APlayerCharacter* Interactor) override;
    virtual void OnFocus_Implementation(APlayerCharacter* Interactor) override;
    virtual void OnEndFocus_Implementation(APlayerCharacter* Interactor) override;
};
