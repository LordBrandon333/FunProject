#include "HUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "PlayerCharacter.h"
#include "HealthComponent.h"
#include "StaminaComponent.h"
#include "NeedComponent.h"
#include "TemperatureComponent.h"

void UHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Optional auto-wire (auskommentiert lassen, wenn du explizit Initializer callst)
    /*
    if (!Player.IsValid())
    {
        if (APawn* Pawn = GetOwningPlayerPawn())
        {
            InitializeFromCharacter(Cast<APlayerCharacter>(Pawn));
        }
    }
    */
}

void UHUDWidget::NativeDestruct()
{
    // Sicher deregistrieren (nur wenn gültig)
    if (Health.IsValid()) { Health->OnHealthChanged.RemoveAll(this); }
    if (Stamina.IsValid()) { Stamina->OnStaminaChanged.RemoveAll(this); }
    if (Hunger.IsValid()) { Hunger->OnNeedChanged.RemoveAll(this); }
    if (Thirst.IsValid()) { Thirst->OnNeedChanged.RemoveAll(this); }
    if (Temperature.IsValid())
    {
        Temperature->OnAmbientEffectiveChanged.RemoveAll(this);
        Temperature->OnCoreTempChanged.RemoveAll(this);
    }

    Super::NativeDestruct();
}

void UHUDWidget::InitializeFromCharacter(APlayerCharacter* InCharacter)
{
    if (!InCharacter) return;
    if (Player.Get() == InCharacter) return;

    // Wenn bereits gebunden: alte Bindings lösen
    NativeDestruct();

    Player = InCharacter;

    Health = Player->FindComponentByClass<UHealthComponent>();
    Stamina = Player->FindComponentByClass<UStaminaComponent>();
    Temperature = Player->FindComponentByClass<UTemperatureComponent>();

    // Needs: beide gezielt suchen (falls du spezialisierte Komponenten hast)
    Hunger = nullptr;
    Thirst = nullptr;
    if (UHungerComponent* H = Player->FindComponentByClass<UHungerComponent>()) Hunger = H;
    if (UThirstComponent* T = Player->FindComponentByClass<UThirstComponent>()) Thirst = T;

    // Bind events (nur wenn vorhanden)
    if (Health.IsValid())
        Health->OnHealthChanged.AddDynamic(this, &UHUDWidget::OnHealthChanged);

    if (Stamina.IsValid())
        Stamina->OnStaminaChanged.AddDynamic(this, &UHUDWidget::OnStaminaChanged);

    if (Hunger.IsValid())
        Hunger->OnNeedChanged.AddDynamic(this, &UHUDWidget::OnNeedChanged);

    if (Thirst.IsValid())
        Thirst->OnNeedChanged.AddDynamic(this, &UHUDWidget::OnNeedChanged);

    if (Temperature.IsValid())
    {
        Temperature->OnAmbientEffectiveChanged.AddDynamic(this, &UHUDWidget::OnAmbientTempChanged);
        Temperature->OnCoreTempChanged.AddDynamic(this, &UHUDWidget::OnBodyTempChanged);

        // Sofort initiale Anzeige:
        UpdateTempTexts();
    }

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

void UHUDWidget::OnHealthChanged(UHealthComponent* Comp, float /*OldV*/, float NewV, float /*Delta*/, AActor* /*Instigator*/)
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

    if (Comp == Hunger.Get())
        SetBar(HungerBar, HungerText, NewV, Comp->GetMax());
    else if (Comp == Thirst.Get())
        SetBar(ThirstBar, ThirstText, NewV, Comp->GetMax());
}

// NEU: Event liefert (AmbientC, HeatC). Wir zeigen *Effective* = Ambient + Heat.
void UHUDWidget::OnAmbientTempChanged(float AmbientC, float HeatContributionC)
{
    if (AmbientTempText)
    {
        const float Effective = AmbientC + HeatContributionC;
        AmbientTempText->SetText(FormatDegC(Effective));
    }
}

void UHUDWidget::OnBodyTempChanged(float NewCoreC)
{
    if (BodyTempText)
    {
        BodyTempText->SetText(FormatDegC(NewCoreC));
    }
}

void UHUDWidget::UpdateTempTexts()
{
    if (!Temperature.IsValid()) return;

    // Initiale Anzeige: wir haben in der Component Caches (LastAmbientC, LastHeatContributionC)
    if (AmbientTempText)
    {
        const float Effective = Temperature->LastAmbientC + Temperature->LastHeatContributionC;
        AmbientTempText->SetText(FormatDegC(Effective));
    }

    if (BodyTempText)
    {
        BodyTempText->SetText(FormatDegC(Temperature->GetBodyTempC()));
    }
}

FText UHUDWidget::FormatDegC(float Value)
{
    // Unicode-robust, vermeidet Compiler-Warnungen/Encoding-Probleme
    return FText::FromString(FString::Printf(TEXT("%.1f \u00B0C"), Value));
}

void UHUDWidget::SetBar(UProgressBar* Bar, UTextBlock* Txt, float Current, float Max)
{
    if (Bar && Max > 0.f)
        Bar->SetPercent(FMath::Clamp(Current / Max, 0.f, 1.f));

    if (Txt)
        Txt->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Current, Max)));
}
