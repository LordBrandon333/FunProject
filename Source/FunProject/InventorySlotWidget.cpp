// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySlotWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "ItemData.h"

void UInventorySlotWidget::Setup(UInventoryComponent* InInventory, int32 InSlotIndex, UItemData* InItem, int32 InCount)
{
	Inventory = InInventory;
	SlotIndex = InSlotIndex;
	Item = InItem;
	Count = InCount;
	RefreshVisuals();
}

void UInventorySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (ClickButton)
	{
		ClickButton->OnClicked.AddDynamic(this, &UInventorySlotWidget::HandleClicked);
	}
	RefreshVisuals();
}

void UInventorySlotWidget::HandleClicked()
{
	OnSlotClicked.Broadcast(SlotIndex);
}

void UInventorySlotWidget::RefreshVisuals()
{
	if (Icon)
	{
		if (Item && Item->Icon)
		{
			Icon->SetBrushFromTexture(Item->Icon);
			Icon->SetOpacity(1.f);
		}
		else
		{
			Icon->SetBrushFromTexture(nullptr);
			Icon->SetOpacity(0.15f);
		}
	}

	if (CountText)
	{
		if (Item && Count > 1)
		{
			CountText->SetText(FText::AsNumber(Count));
			CountText->SetOpacity(1.f);
		}
		else
		{
			CountText->SetText(FText::GetEmpty());
			CountText->SetOpacity(0.5f);
		}
	}
}


