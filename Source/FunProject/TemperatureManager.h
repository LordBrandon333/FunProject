// TemperatureManager.h
#pragma once
#include "Subsystems/WorldSubsystem.h"
#include "TemperatureRules.h"
#include "TemperatureManager.generated.h"

class UHeatSourceComponent;

USTRUCT(BlueprintType)
struct FTemperatureQuery
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float WindSpeed = 0.f;  // m/s (optional, falls du später Wetter einbaust)
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Wetness01 = 0.f;  // 0..1
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsIndoors = false; // spätere Nutzung möglich
};

UCLASS()
class FUNPROJECT_API UTemperatureManager : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Temperature")
    void SetTimeOfDayHours(float Hours); // vom WorldTimeManager gesetzt

    UFUNCTION(BlueprintPure, Category = "Temperature")
    float GetAmbientTemperatureC(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintPure, Category = "Temperature")
    float ComputeHeatSourceContributionC(const FVector& WorldLocation) const;

    // Eine Integrationsfunktion, die Core/Skin auf Basis von Umgebung etc. fortschreibt
    void ComputeBodyStep(
        float& InOutCoreTempC,
        float& InOutSkinTempC,
        float InsulationClo,
        float Metabolic01,
        float Wetness01,
        float EffectiveAmbientC,
        float DeltaSeconds) const;

    void RegisterHeatSource(UHeatSourceComponent* Source);
    void UnregisterHeatSource(UHeatSourceComponent* Source);

    const UTemperatureRules* GetRules() const { return Rules; }
    void SetRules(UTemperatureRules* NewRules) { Rules = NewRules; }

private:
    UPROPERTY() TObjectPtr<UTemperatureRules> Rules = nullptr;
    UPROPERTY() TArray<TWeakObjectPtr<UHeatSourceComponent>> HeatSources;

    float CachedTimeOfDayHours = 12.f;

    float SampleDiurnalDeltaC(float Hours) const;
};
