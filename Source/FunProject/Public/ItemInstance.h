// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemActionBase.h"
#include "ItemDataStructs.h"
#include "ItemFragment.h"
#include "ItemInstance.generated.h"


UCLASS(BlueprintType)
class FUNPROJECT_API UItemInstance : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UDataTable> ItemDataTable = nullptr;

	UPROPERTY(BlueprintReadOnly)
	FName ItemID;

	UPROPERTY(BlueprintReadOnly)
	FItemDataRow CachedData;

	UPROPERTY(Instanced, BlueprintReadOnly)
	TArray<TObjectPtr<UItemFragment>> Fragments;

	UPROPERTY(Instanced, BlueprintReadOnly)
	TMap<EItemActionSlot, TObjectPtr<UItemActionBase>> Actions;

	UPROPERTY(BlueprintReadOnly)
	int32 StackCount = 1;

public:
	UFUNCTION(BlueprintCallable)
	bool InitializeFromDataTable(UDataTable* InTable, FName InItemID);

	template<typename T> T* FindFragment() const
	{
		for (UItemFragment* F : Fragments)
			if (T* As = Cast<T>(F)) return As;
		return nullptr;
	}

	UFUNCTION(BlueprintCallable)
	EItemActionResult UseSlot(EItemActionSlot Slot, AActor* User);
};
