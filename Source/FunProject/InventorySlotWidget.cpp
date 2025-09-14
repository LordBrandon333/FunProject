#include "InventorySlotWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "ItemData.h"
#include "InventoryComponent.h"

void UInventorySlotWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (ClickButton)
    {
        ClickButton->OnClicked.AddDynamic(this, &UInventorySlotWidget::HandleClicked);
        ClickButton->SetClickMethod(EButtonClickMethod::MouseDown);
    }
    RefreshVisuals();
}

void UInventorySlotWidget::Setup(UInventoryComponent* InInventory, int32 InSlotIndex, UItemData* InItem, int32 InCount)
{
    Inventory = InInventory;
    SlotIndex = InSlotIndex;
    Item = InItem;
    Count = InCount;
    RefreshVisuals();
}

void UInventorySlotWidget::HandleClicked()
{
    OnSlotClicked.Broadcast(SlotIndex);
}

void UInventorySlotWidget::SetSelected(bool bInSelected)
{
    bSelected = bInSelected;
    if (SelectionBorder)
    {
        SelectionBorder->SetBrushColor(bSelected ? SelectedTint : FLinearColor(0, 0, 0, 0));
        SelectionBorder->SetRenderOpacity(bSelected ? 1.f : 0.f);
    }
    else if (Icon)
    {
        Icon->SetRenderScale(bSelected ? FVector2D(1.05f, 1.05f) : FVector2D(1.f, 1.f));
    }
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
