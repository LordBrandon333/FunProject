#include "HUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "PlayerCharacter.h"
#include "HealthComponent.h"
#include "StaminaComponent.h"
#include "NeedComponent.h"

void UHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // If not initialized manually, try to auto-wire from owning pawn
    /*if (!Player.IsValid())
    {
        if (APawn* Pawn = GetOwningPlayerPawn())
        {
            InitializeFromCharacter(Cast<APlayerCharacter>(Pawn));
        }
    }*/
}

void UHUDWidget::NativeDestruct()
{
    // (Optional) Remove dynamic bindings if you add explicit RemoveDynamic calls later
    Super::NativeDestruct();
}

void UHUDWidget::InitializeFromCharacter(APlayerCharacter* InCharacter)
{
    if (!InCharacter) return;
    if (Player.Get() == InCharacter) return;

    Player = InCharacter;

    Health = Player->FindComponentByClass<UHealthComponent>();
    Stamina = Player->FindComponentByClass<UStaminaComponent>();
    // Needs: typed pointers exist (UHungerComponent/UThirstComponent), but both derive UNeedComponent – reicht hier:
    Hunger = Player->FindComponentByClass<UNeedComponent>(); // first found; we’ll reassign properly below
    Thirst = nullptr;

    // Better: find exact types
    if (UHungerComponent* H = Player->FindComponentByClass<UHungerComponent>()) Hunger = H;
    if (UThirstComponent* T = Player->FindComponentByClass<UThirstComponent>()) Thirst = T;

    // Bind events
    if (Health.IsValid())
        Health->OnHealthChanged.AddDynamic(this, &UHUDWidget::OnHealthChanged);
    if (Stamina.IsValid())
        Stamina->OnStaminaChanged.AddDynamic(this, &UHUDWidget::OnStaminaChanged);
    if (Hunger.IsValid())
        Hunger->OnNeedChanged.AddDynamic(this, &UHUDWidget::OnNeedChanged);
    if (Thirst.IsValid())
        Thirst->OnNeedChanged.AddDynamic(this, &UHUDWidget::OnNeedChanged);

    RefreshAll();
}

void UHUDWidget::RefreshAll()
{
    if (Health.IsValid())
        SetBar(HealthBar, HealthText, Health->GetHealth(), Health->GetMaxHealth());

    if (Stamina.IsValid())
        SetBar(StaminaBar, StaminaText, Stamina->GetStamina(), Stamina->GetMaxStamina());

    if (Hunger.IsValid())
        SetBar(HungerBar, HungerText, Hunger->GetCurrent(), Hunger->GetMax());

    if (Thirst.IsValid())
        SetBar(ThirstBar, ThirstText, Thirst->GetCurrent(), Thirst->GetMax());
}

void UHUDWidget::OnHealthChanged(UHealthComponent* Comp, float OldV, float NewV, float Delta, AActor* /*Instigator*/)
{
    SetBar(HealthBar, HealthText, NewV, Comp ? Comp->GetMaxHealth() : 1.f);
}

void UHUDWidget::OnStaminaChanged(UStaminaComponent* Comp, float /*OldV*/, float NewV, float /*Delta*/)
{
    SetBar(StaminaBar, StaminaText, NewV, Comp ? Comp->GetMaxStamina() : 1.f);
}

void UHUDWidget::OnNeedChanged(UNeedComponent* Comp, float /*OldV*/, float NewV, float /*Delta*/)
{
    if (!Comp) return;

    // Decide which bar this is (by pointer compare)
    if (Comp == Hunger.Get())
        SetBar(HungerBar, HungerText, NewV, Comp->GetMax());
    else if (Comp == Thirst.Get())
        SetBar(ThirstBar, ThirstText, NewV, Comp->GetMax());
}

void UHUDWidget::SetBar(UProgressBar* Bar, UTextBlock* Txt, float Current, float Max)
{
    if (Bar && Max > 0.f)
        Bar->SetPercent(FMath::Clamp(Current / Max, 0.f, 1.f));

    if (Txt)
        Txt->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Current, Max)));
}
