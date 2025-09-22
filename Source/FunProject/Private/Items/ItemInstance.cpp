#include "Items/ItemInstance.h"
#include "Items/ItemFragment.h"
#include "Items/ItemAction.h"


bool UItemInstance::InitializeFromDataTable(UDataTable* InTable, FName InItemID)
{
	ItemDataTable = InTable; ItemID = InItemID;
	if (!ItemDataTable) return false;


	if (const FItemDataRow* Row = ItemDataTable->FindRow<FItemDataRow>(ItemID, TEXT("ItemInit")))
	{
		CachedData = *Row;


		// Fragmente instanziieren
		for (const TSoftClassPtr<UItemFragment>& FragClassSoft : CachedData.FragmentClasses)
		{
			if (UClass* FragClass = FragClassSoft.LoadSynchronous())
			{
				UItemFragment* Frag = NewObject<UItemFragment>(this, FragClass);
				Frag->OwnerInstance = this;
				Frag->InitializeFragment();
				Fragments.Add(Frag);
			}
		}


		// Actions instanziieren & Slots belegen
		for (const FActionBinding& Binding : CachedData.DefaultActions)
		{
			if (UClass* ActionClass = Binding.ActionClass.LoadSynchronous())
			{
				UItemAction* Act = NewObject<UItemAction>(this, ActionClass);
				Act->OwnerInstance = this;
				Actions.Add(Binding.Slot, Act);
			}
		}
		return true;
	}
	return false;
}


EItemActionResult UItemInstance::UseSlot(EItemActionSlot Slot, AActor* User)
{
	if (TObjectPtr<UItemAction>* ActPtr = Actions.Find(Slot))
	{
		if (UItemAction* Act = ActPtr->Get())
		{
			return Act->Execute(User);
		}
	}
	return EItemActionResult::NoOp;
}