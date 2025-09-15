#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TemperatureComponent.generated.h"

class AWorldTimeManager;
class UHealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBodyTempChanged, float, OldC, float, NewC);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FUNPROJECT_API UTemperatureComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UTemperatureComponent();

    UFUNCTION(BlueprintPure, Category = "Temp")
    float GetBodyTempC() const { return BodyTempC; }

    UFUNCTION(BlueprintPure, Category = "Temp")
    float GetAmbientTempC() const { return AmbientTempC; }

    UPROPERTY(BlueprintAssignable, Category = "Temp")
    FOnBodyTempChanged OnBodyTempChanged;

    // einfache Stellschrauben
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temp|Balance")
    float ComfortBodyTempC = 37.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temp|Balance")
    FVector2D SafeBodyTempRangeC = FVector2D(35.5f, 38.5f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temp|Balance")
    float Insulation = 0.3f; // 0..1 (Kleidung): 0 keine, 1 perfekte Isolation

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temp|Balance")
    float Wetness = 0.f;     // 0..1

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temp|Balance")
    float WindExposure = 0.2f; // 0..1 (draußen 1, drinnen 0)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temp|Tick")
    float UpdateInterval = 0.5f; // s

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temp|Balance")
    float ElevationLapseRateCPerMeter = -0.0065f; // -0.65 °C pro 100 m

    // Diurnaler Temperaturanteil (0..1 -> Δ°C). Optional – kann leer sein.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Temp|Curves")
    TObjectPtr<UCurveFloat> DiurnalCurve = nullptr;

protected:
    virtual void BeginPlay() override;

private:
    FTimerHandle TickHandle;

    float BodyTempC = 37.f;
    float AmbientTempC = 15.f;

    TWeakObjectPtr<AWorldTimeManager> TimeMgr;
    TWeakObjectPtr<UHealthComponent>  Health;

    void UpdateOnce();

    float ComputeAmbientAt(const FVector& Loc) const;
    float SumHeatSourcesAt(const FVector& Loc) const;
    float ComputeMetabolicHeat() const; // Bewegung/Anstrengung → +°C

    void ApplyHealthEffects(float OldC, float NewC);
};
