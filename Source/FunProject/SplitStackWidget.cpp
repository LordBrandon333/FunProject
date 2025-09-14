#include "SplitStackWidget.h"
#include "Components/TextBlock.h"
#include "Components/Slider.h"
#include "Components/Button.h"
#include "InventoryWidget.h"

void USplitStackWidget::Init(UInventoryWidget* InOwner, int32 InSlotIndex, int32 InMaxCount)
{
    Owner = InOwner; SlotIndex = InSlotIndex;
    MaxCount = FMath::Clamp(InMaxCount, 1, InMaxCount);
    Chosen = FMath::Clamp(Chosen, 1, MaxCount);

    if (TitleText)  TitleText->SetText(FText::FromString(TEXT("Stack aufteilen")));
    if (AmountSlider)
    {
        AmountSlider->SetMinValue(1.f);
        AmountSlider->SetMaxValue((float)MaxCount);
        AmountSlider->SetStepSize(1.f / FMath::Max(1, MaxCount));
        AmountSlider->SetValue((float)Chosen);
    }
    if (AmountText) AmountText->SetText(FText::AsNumber(Chosen));

    if (OkButton)     OkButton->OnClicked.AddDynamic(this, &USplitStackWidget::OnOk);
    if (CancelButton) CancelButton->OnClicked.AddDynamic(this, &USplitStackWidget::OnCancel);
    if (AmountSlider) AmountSlider->OnValueChanged.AddDynamic(this, &USplitStackWidget::OnSliderChanged);
}

void USplitStackWidget::OnSliderChanged(float NewValue)
{
    Chosen = FMath::Clamp(FMath::RoundToInt(NewValue), 1, MaxCount);
    if (AmountText) AmountText->SetText(FText::AsNumber(Chosen));
}

void USplitStackWidget::OnOk()
{
    if (Owner.IsValid()) Owner->ConfirmSplit(SlotIndex, Chosen);
    RemoveFromParent();
}

void USplitStackWidget::OnCancel()
{
    if (Owner.IsValid()) Owner->CancelSplit();
    RemoveFromParent();
}
