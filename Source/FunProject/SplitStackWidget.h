#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SplitStackWidget.generated.h"

class UTextBlock; class USlider; class UButton;

UCLASS()
class FUNPROJECT_API USplitStackWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    void Init(class UInventoryWidget* InOwner, int32 InSlotIndex, int32 InMaxCount);

protected:
    UPROPERTY(meta = (BindWidget)) UTextBlock* TitleText = nullptr;
    UPROPERTY(meta = (BindWidget)) UTextBlock* AmountText = nullptr;
    UPROPERTY(meta = (BindWidget)) USlider* AmountSlider = nullptr;
    UPROPERTY(meta = (BindWidget)) UButton* OkButton = nullptr;
    UPROPERTY(meta = (BindWidget)) UButton* CancelButton = nullptr;

    UFUNCTION() void OnSliderChanged(float NewValue);
    UFUNCTION() void OnOk();
    UFUNCTION() void OnCancel();

private:
    TWeakObjectPtr<class UInventoryWidget> Owner;
    int32 SlotIndex = INDEX_NONE;
    int32 MaxCount = 0;      // maximal abspaltbar (z.B. SlotCount-1)
    int32 Chosen = 1;        // aktuelle Auswahl
};
