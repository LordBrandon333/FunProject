// TemperatureManager.cpp
#include "TemperatureManager.h"
#include "TemperatureSettings.h"
#include "HeatSourceComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void UTemperatureManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Default Rules aus Project Settings laden
    if (const auto* Settings = GetDefault<UTemperatureSettings>())
    {
        if (Settings->DefaultRules.IsValid())
        {
            UObject* Loaded = Settings->DefaultRules.TryLoad();
            Rules = Cast<UTemperatureRules>(Loaded);
        }
    }

    // Fallback, falls kein Asset gesetzt wurde
    if (!Rules)
    {
        Rules = NewObject<UTemperatureRules>(this);
    }
}

void UTemperatureManager::Deinitialize()
{
    HeatSources.Reset();
    Super::Deinitialize();
}

void UTemperatureManager::SetTimeOfDayHours(float Hours)
{
    // clamp auf [0,24)
    CachedTimeOfDayHours = FMath::Fmod(FMath::Max(0.f, Hours), 24.f);
}

float UTemperatureManager::SampleDiurnalDeltaC(float Hours) const
{
    if (Rules && Rules->DiurnalTempCurve)
    {
        return Rules->DiurnalTempCurve->GetFloatValue(Hours);
    }
    return 0.f;
}

float UTemperatureManager::GetAmbientTemperatureC(const FVector& WorldLocation) const
{
    const float Base = Rules ? Rules->BaseAmbientC : 20.f;
    const float Diurnal = SampleDiurnalDeltaC(CachedTimeOfDayHours);

    float AltitudeDelta = 0.f;
    if (Rules)
    {
        const float DzHundredsOfMeters = (WorldLocation.Z - Rules->SeaLevelZ) / 10000.f; // in 100m
        AltitudeDelta = DzHundredsOfMeters * Rules->AltitudeLapseRateCPer100m;
    }

    return Base + Diurnal + AltitudeDelta;
}

float UTemperatureManager::ComputeHeatSourceContributionC(const FVector& WorldLocation) const
{
    float Sum = 0.f;
    int32 Count = 0;
    for (const TWeakObjectPtr<UHeatSourceComponent>& Weak : HeatSources)
    {
        const UHeatSourceComponent* S = Weak.Get();
        if (!S || !S->bEnabled) continue;
        ++Count;

        const float Dist = FVector::Dist(WorldLocation, S->GetComponentLocation());
        if (Dist >= S->Radius || S->Radius <= 1.f) continue;

        const float Alpha = 1.f - (Dist / S->Radius);
        const float Fall = FMath::Pow(FMath::Clamp(Alpha, 0.f, 1.f), S->FalloffExponent);
        Sum += S->TemperatureDeltaC * Fall;
    }
    UE_LOG(LogTemp, Log, TEXT("[Temp] HeatSources=%d  HeatC=%.2f"), Count, Sum);
    return Sum;
}

void UTemperatureManager::ComputeBodyStep(
    float& Core, float& Skin, float InsulationClo, float Metabolic01, float Wetness01,
    float AmbientC, float Dt) const
{
    if (!Rules || Dt <= KINDA_SMALL_NUMBER) return;

    // Effektive Umgebung inkl. HeatSources wird außerhalb addiert
    // Skin -> Ambient Annäherung
    const float Insulation = FMath::Max(0.1f, InsulationClo); // nie 0
    const float ExchangeMult = 1.f + Rules->WetnessAmplifier * FMath::Clamp(Wetness01, 0.f, 1.f);
    const float SkinTau = FMath::Max(1.f, Rules->SkinTimeConstantSeconds * Insulation / ExchangeMult);

    const float dSkin = (AmbientC - Skin) * (Dt / SkinTau);
    Skin += dSkin;

    // Core ↔ Skin + Metabolik
    const float CoreTau = FMath::Max(1.f, Rules->CoreTimeConstantSeconds);
    const float dCoreExchange = (Skin - Core) * (Dt / CoreTau);

    const float HeatGainPerSec = FMath::Lerp(
        Rules->RestingHeatGainCPerMinute,
        Rules->SprintingHeatGainCPerMinute,
        FMath::Clamp(Metabolic01, 0.f, 1.f)) / 60.f;

    Core += dCoreExchange + HeatGainPerSec * Dt;

    // Hard-Limits
    Core = FMath::Clamp(Core, Rules->MinCoreTempC, Rules->MaxCoreTempC);
}

void UTemperatureManager::RegisterHeatSource(UHeatSourceComponent* Source)
{
    HeatSources.AddUnique(Source);
}

void UTemperatureManager::UnregisterHeatSource(UHeatSourceComponent* Source)
{
    HeatSources.Remove(Source);
}
