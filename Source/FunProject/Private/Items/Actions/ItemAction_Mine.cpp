#include "Items/Actions/ItemAction_Mine.h"
#include "Items/ItemInstance.h"
#include "Items/Fragments/ItemFragment_Tool.h"
#include "Items/Fragments/ItemFragment_Durability.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"

UItemAction_Mine::UItemAction_Mine()
{
	RequiredFragments = { UItemFragment_Tool::StaticClass(), UItemFragment_Durability::StaticClass() };
}

EItemActionResult UItemAction_Mine::Execute_Implementation(AActor* User)
{
	if (!OwnerInstance || !User) return EItemActionResult::Failed;
	
	auto* Tool = OwnerInstance->FindFragment<UItemFragment_Tool>();
	auto* Dur = OwnerInstance->FindFragment<UItemFragment_Durability>();
	if (!Tool || !Dur || !Dur->CanUse()) return EItemActionResult::Failed;
	
	// Beispiel: einfacher LineTrace (Server-seitig), echte Implementierung kann Montage/Cooldown etc. enthalten.
	FVector Start = User->GetActorLocation();
	FVector End = Start + User->GetActorForwardVector() * Range;
	
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(ItemActionMine), false, User);
	bool bHit = User->GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
	
	if (!bHit || !Hit.GetActor()) return EItemActionResult::NoOp;
	
	// Ziel-Tags abrufen (hier exemplarisch über ein Interface oder Component gedacht)
	FGameplayTagContainer TargetTags; // TODO: von Hit.Actor sammeln
	
	if (!Tool->CanHit(TargetTags))
		return EItemActionResult::NoOp;
	
	// Beispielhafte Nachricht an ResourceNode (Pseudo-API)
	// IHarvestable::Execute_ApplyHit(Hit.GetActor(), Tool->HarvestPower, User);
	
	const bool bBroke = Dur->ApplyUse(DurabilityCostPerHit);
	if (bBroke)
	{
		// TODO: Feedback/Destroy-Flow (OwnerInstance kennzeichnet sich als Broken etc.)
	}
	
	return EItemActionResult::Performed;
}
