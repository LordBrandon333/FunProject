#pragma once
#include "Components/ActorComponent.h"
#include "TemperatureComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCoreTemperatureChanged, float, NewCoreC);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmbientEffectiveChanged, float, AmbientC, float, HeatContributionC);

UCLASS(ClassGroup = (Temperature), meta = (BlueprintSpawnableComponent))
class FUNPROJECT_API UTemperatureComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    float CoreTempC = 37.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    float SkinTempC = 33.f;

    // 1.0 ~ normale Kleidung; höher = mehr Isolierung
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float InsulationClo = 1.0f;

    // 0..1 (0 = ruhen; 1 = sprinten)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float Metabolic01 = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float Wetness01 = 0.f;

    // Debug/HUD-Caches (Basis und Heat getrennt)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug", meta = (AllowPrivateAccess = "true"))
    float LastAmbientC = 20.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug", meta = (AllowPrivateAccess = "true"))
    float LastHeatContributionC = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug", meta = (AllowPrivateAccess = "true"))
    float LastOperativeAmbientC = 20.f;

    // Events
    UPROPERTY(BlueprintAssignable)
    FOnCoreTemperatureChanged OnCoreTempChanged;

    UPROPERTY(BlueprintAssignable)
    FOnAmbientEffectiveChanged OnAmbientEffectiveChanged;

    UTemperatureComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === Getter fürs HUD ===
    UFUNCTION(BlueprintPure, Category = "Temperature")
    float GetAmbientTempC() const { return LastAmbientC; }                     // Basis (ohne Heat)

    UFUNCTION(BlueprintPure, Category = "Temperature")
    float GetHeatContributionC() const { return LastHeatContributionC; }       // nur Heat

    UFUNCTION(BlueprintPure, Category = "Temperature")
    float GetEffectiveAmbientTempC() const { return LastAmbientC + LastHeatContributionC; } // Basis+Heat

    UFUNCTION(BlueprintPure, Category = "Temperature")
    float GetBodyTempC() const { return CoreTempC; }                            // Core

    UFUNCTION(BlueprintPure, Category = "Temperature")
    float GetOperativeAmbientTempC() const { return LastOperativeAmbientC; }

private:
    TWeakObjectPtr<class UTemperatureManager> Manager;
    float LastBroadcastCore = TNumericLimits<float>::Lowest();
};
