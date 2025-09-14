#include "ItemPickup.h"
#include "Components/StaticMeshComponent.h"
#include "ItemData.h"
#include "InventoryComponent.h"
#include "PlayerCharacter.h"
#include "Kismet/KismetSystemLibrary.h"

AItemPickup::AItemPickup()
{
    PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(Mesh);
    // Kollision so, dass dein Interact-Trace (Visibility) blockiert wird:
    Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
}

bool AItemPickup::CanInteract_Implementation(APlayerCharacter* /*Interactor*/)
{
    return Item != nullptr && Count > 0;
}

void AItemPickup::Interact_Implementation(APlayerCharacter* Interactor)
{
    if (!Interactor || !Item || Count <= 0) return;

    if (UInventoryComponent* Inv = Interactor->FindComponentByClass<UInventoryComponent>())
    {
        const int32 Added = Inv->TryAddItem(Item, Count);
        Count -= Added;

        if (Added > 0)
        {
            const FString Name = Item->ItemId.IsNone() ? Item->GetName() : Item->ItemId.ToString();
            UKismetSystemLibrary::PrintString(this,
                FString::Printf(TEXT("Picked up %d x %s"), Added, *Name),
                true, false, FLinearColor::Green, 1.5f);
        }

        if (Count <= 0)
        {
            Destroy();
        }
    }
}

void AItemPickup::OnFocus_Implementation(APlayerCharacter* /*Interactor*/)
{
    if (Mesh) Mesh->SetRenderCustomDepth(true);
}

void AItemPickup::OnEndFocus_Implementation(APlayerCharacter* /*Interactor*/)
{
    if (Mesh) Mesh->SetRenderCustomDepth(false);
}
