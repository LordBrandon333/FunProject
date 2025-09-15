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
    float MinCoreTempC = 28.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body")
    float MaxCoreTempC = 42.5f;

    // Zeitkonstanten (je kleiner, desto schneller Annäherung)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body")
    float CoreTimeConstantSeconds = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body")
    float SkinTimeConstantSeconds = 90.f;

    // Wind-/Nässe-Einfluss (dimensionslos)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body")
    float WindChillCoefficient = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body")
    float WetnessAmplifier = 3.0f;

    // Metabolische Wärmezufuhr (in °C pro Minute; wird auf Core addiert)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body")
    float RestingHeatGainCPerMinute = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body")
    float SprintingHeatGainCPerMinute = 0.20f;
};
