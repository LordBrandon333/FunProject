// HeatSourceComponent.h
#pragma once
#include "Components/SceneComponent.h"
#include "HeatSourceComponent.generated.h"

UCLASS(ClassGroup = (Temperature), meta = (BlueprintSpawnableComponent))
class FUNPROJECT_API UHeatSourceComponent : public USceneComponent
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heat")
    float TemperatureDeltaC = 15.f;   // Peak am Zentrum

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heat")
    float Radius = 500.f;

    // 1 = linear, 2 = quadratisch (schnelleres Abklingen)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heat")
    float FalloffExponent = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heat")
    bool bEnabled = true;

    virtual void OnRegister() override;
    virtual void OnUnregister() override;
};
