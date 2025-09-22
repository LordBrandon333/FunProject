// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemInstance.h"
#include "ItemFragment.h"
#include "ItemActionBase.h"


bool UItemInstance::InitializeFromDataTable(UDataTable* InTable, FName InItemID)
{
	ItemDataTable = InTable; ItemID = InItemID;
	if (!ItemDataTable) return false;

	if (const FItemDataRow* Row = ItemDataTable->FindRow<FItemDataRow>(ItemID, TEXT("ItemInit")))
	{
		CachedData = *Row;

		// Initialize Fragments
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

		// Initialize Actions
		for (const FActionBinding& Binding : CachedData.DefaultActions)
		{
			if (UClass* ActionClass = Binding.ActionClass.LoadSynchronous())
			{
				UItemActionBase* Act = NewObject<UItemActionBase>(this, ActionClass);
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
	if (TObjectPtr<UItemActionBase>* ActPtr = Actions.Find(Slot))
	{
		if (UItemActionBase* Act = ActPtr->Get())
		{
			return Act->Execute(User);
		}
	}
	return EItemActionResult::NoOp;
}
