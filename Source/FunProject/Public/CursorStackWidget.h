// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CursorStackWidget.generated.h"

class UImage;
class UTextBlock;
class UItemData;


UCLASS()
class FUNPROJECT_API UCursorStackWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Cursor")
	void SetStack(UItemData* Item, int32 Count);

	UFUNCTION(BlueprintCallable, Category = "Cursor")
	void Clear();

protected:
	UPROPERTY(meta = (BindWidget)) UImage* Icon = nullptr;
	UPROPERTY(meta = (BindWidget)) UTextBlock* CountText = nullptr;
};
