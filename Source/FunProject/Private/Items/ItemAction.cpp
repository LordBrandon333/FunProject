
#include "Items/ItemAction.h"
#include "Items/ItemInstance.h"
#include "Items/ItemFragment.h"
#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif
void UItemAction::PostEditChangeProperty(FPropertyChangedEvent& E)
{
	Super::PostEditChangeProperty(E);
	// Build human-readable summary of required fragments
	TArray<FString> Names;
	for (const TSubclassOf<UItemFragment>& Cls : RequiredFragments)
	{
		if (const UClass* C = Cls.Get()) Names.Add(C->GetName());
	}
	const FString Line = Names.Num() ? FString::Join(Names, TEXT(", ")) : TEXT("— (none)");
	RequirementsSummary = FText::FromString(FString::Printf(TEXT("Required fragments: %s"), *Line));
}

EDataValidationResult UItemAction::IsDataValid(TArray<FText>& Errors) const
{
	EDataValidationResult Result = EDataValidationResult::Valid;
	if (!OwnerInstance) return Result; // OwnerInstance may be null in asset defaults


	for (const TSubclassOf<UItemFragment>& Cls : RequiredFragments)
	{
		if (!Cls) continue;
		if (!OwnerInstance->FindFragmentByClass(Cls))
		{
			Result = EDataValidationResult::Invalid;
			Errors.Add(FText::FromString(FString::Printf(TEXT("[%s] missing on ItemInstance"), *Cls->GetName())));
		}
	}
	return Result;
}
