// TemperatureComponent.h
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State") float CoreTempC = 37.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State") float SkinTempC = 33.f;

    // 1.0 ~ normale Kleidung; höher = mehr Isolierung
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character") float InsulationClo = 1.0f;

    // 0..1 (0 = ruhen; 1 = sprinten). Setz das z.B. im Character abhängig von Movement.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character") float Metabolic01 = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character") float Wetness01 = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug", meta = (AllowPrivateAccess = "true")) float LastAmbientC = 20.f;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug", meta = (AllowPrivateAccess = "true")) float LastHeatContributionC = 0.f;

    UPROPERTY(BlueprintAssignable) FOnCoreTemperatureChanged OnCoreTempChanged;
    UPROPERTY(BlueprintAssignable) FOnAmbientEffectiveChanged OnAmbientEffectiveChanged;

    // Alias
    UPROPERTY(BlueprintAssignable) FOnCoreTemperatureChanged OnBodyTempChanged;               
    UPROPERTY(BlueprintAssignable) FOnAmbientEffectiveChanged OnAmbientTempChanged;

    UTemperatureComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintPure, Category = "Temperature")
    float GetAmbientTempC() const { return LastAmbientC; }

    // Historisch hieß es "BodyTemp" – wir geben Core zurück.
    UFUNCTION(BlueprintPure, Category = "Temperature")
    float GetBodyTempC() const { return CoreTempC; }

private:
    TWeakObjectPtr<class UTemperatureManager> Manager;
    float LastBroadcastCore = TNumericLimits<float>::Lowest();
};
