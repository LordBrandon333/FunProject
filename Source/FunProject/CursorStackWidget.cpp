#include "CursorStackWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "ItemData.h"

void UCursorStackWidget::SetStack(UItemData* Item, int32 Count)
{
    if (!Icon || !CountText) return;

    if (Item && Item->Icon && Count > 0)
    {
        Icon->SetBrushFromTexture(Item->Icon);
        Icon->SetOpacity(1.f);
        CountText->SetText(Count > 1 ? FText::AsNumber(Count) : FText::GetEmpty());
        SetVisibility(ESlateVisibility::HitTestInvisible);
    }
    else
    {
        Clear();
    }
}

void UCursorStackWidget::Clear()
{
    if (Icon)
    {
        Icon->SetBrushFromTexture(nullptr);
        Icon->SetOpacity(0.f);
    }
    if (CountText) CountText->SetText(FText::GetEmpty());
    SetVisibility(ESlateVisibility::Collapsed);
}
