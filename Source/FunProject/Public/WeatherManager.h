#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "WeatherManager.generated.h"

USTRUCT(BlueprintType)
struct FWeatherSample
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float WindMS = 0.f;     // m/s
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Rain01 = 0.f;     // 0..1
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Humidity01 = 0.5f;// 0..1
};

UCLASS()
class FUNPROJECT_API UWeatherManager : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    // Simpler globaler Zustand (später gerne Felder/Zonen/Karten)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float BaseWindMS = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float BaseRain01 = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float BaseHumidity01 = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float IndoorsWindMultiplier = 0.1f; // Wind stark gedämpft indoor

    // Zonen-Skalen einrechnen
    UFUNCTION(BlueprintCallable, Category = "Weather")
    FWeatherSample Sample(const FVector& WorldLocation, bool bIsIndoors, float WindScale) const
    {
        FWeatherSample S;
        S.WindMS = BaseWindMS * WindScale * (bIsIndoors ? IndoorsWindMultiplier : 1.f);
        S.Rain01 = bIsIndoors ? 0.f : BaseRain01; // drinnen regnet's nicht
        S.Humidity01 = BaseHumidity01;
        return S;
    }
};
