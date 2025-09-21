#include "ItemAction.h"
#include "HealthComponent.h"
#include "NeedComponent.h"
#include "StatusEffectsComponent.h"
#include "StatusEffectData.h"

bool UItemAction::Execute_Implementation(AActor* /*OwnerActor*/) { return false; }

// --- Heal ---
bool UItemAction_Heal::Execute_Implementation(AActor* OwnerActor)
{
    if (!OwnerActor || Amount <= 0.f) return false;
    if (UHealthComponent* HC = OwnerActor->FindComponentByClass<UHealthComponent>())
    {
        HC->Heal(Amount);
        return true;
    }
    return false;
}

// --- Need Delta ---
bool UItemAction_NeedDelta::Execute_Implementation(AActor* OwnerActor)
{
    if (!OwnerActor || Amount == 0.f || !TargetNeed) return false;

    UClass* NeedClass = *TargetNeed; // aus TSubclassOf<UActorComponent> -> UClass*
    if (!NeedClass) return false;

    if (UActorComponent* C = OwnerActor->GetComponentByClass(NeedClass))
    {
        if (UNeedComponent* Need = Cast<UNeedComponent>(C))
        {
            if (Amount > 0.f) Need->Add(Amount);
            else              Need->Consume(-Amount);
            return true;
        }
    }
    return false;
}

// --- Apply StatusEffect ---
bool UItemAction_ApplyEffect::Execute_Implementation(AActor* OwnerActor)
{
    if (!OwnerActor || !Effect) return false;
    if (UStatusEffectsComponent* Eff = OwnerActor->FindComponentByClass<UStatusEffectsComponent>())
    {
        Eff->ApplyEffect(Effect, SourceId);
        return true;
    }
    return false;
}
