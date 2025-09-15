// TemperatureRules.h
#pragma once
#include "Engine/DataAsset.h"
#include "Curves/CurveFloat.h"
#include "TemperatureRules.generated.h"

UCLASS(BlueprintType)
class FUNPROJECT_API UTemperatureRules : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    // --- Ambient / Umgebung ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ambient")
    float BaseAmbientC = 20.f;

    // Diurnale Schwankung: x = Stunden [0..24], y = Delta °C
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ambient")
    TObjectPtr<UCurveFloat> DiurnalTempCurve = nullptr;

    // Höhen-Temperaturgradient (°C pro 100m)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ambient")
    float AltitudeLapseRateCPer100m = -0.65f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ambient")
    float SeaLevelZ = 0.f;

    // --- Körper / Heat-Exchange ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body")
    float TargetCoreTempC = 37.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body")
    float MinCoreTempC = 32.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body")
    float MaxCoreTempC = 40.5f;

    // Zeitkonstanten (je kleiner, desto schneller Annäherung)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body")
    float CoreTimeConstantSeconds = 1800.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body")
    float SkinTimeConstantSeconds = 120.f;

    // Wind-/Nässe-Einfluss (dimensionslos)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body")
    float WindChillCoefficient = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body")
    float WetnessAmplifier = 2.0f;

    // Metabolische Wärmezufuhr (in °C pro Minute; wird auf Core addiert)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body")
    float RestingHeatGainCPerMinute = 0.06f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body")
    float SprintingHeatGainCPerMinute = 0.35f;

    //Thermoregulation
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body")
    float ThermoregulationGainCPerMinutePerDeg = 0.12f;
    // zusätzliche Heiz-/Kühlrate je °C Abweichung vom Ziel (37 °C)

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body")
    float ThermoregulationMaxAbsCPerMinute = 0.60f;

    // Wie stark Strahlungswärme (HeatSources) die "gefühlte" Umgebung beeinflusst (0..1)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heat Sources")
    float RadiantToOperativeWeight = 0.35f;   // 0.3–0.5 ist gut

    // Kleidung dämpft Strahlung (höhere CLO -> weniger Effekt)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heat Sources")
    float RadiantInsulationFactor = 0.6f;     // 0.4–0.8

    // Obergrenze des zusätzlichen °C-Adders durch Strahlung (gegen unrealistische Werte)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heat Sources")
    float MaxRadiantAddC = 25.f;              // 15–30
};
