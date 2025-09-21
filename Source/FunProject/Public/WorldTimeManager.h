// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldTimeManager.generated.h"

class ADirectionalLight;
class UTemperatureManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeChanged, float, NormalizedTimeOfDay);

UCLASS()
class FUNPROJECT_API AWorldTimeManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AWorldTimeManager();

	virtual void Tick(float DeltaSeconds) override;

    UFUNCTION(BlueprintCallable, Category = "Time")
    void SetTimeScale(float NewScale) { TimeScale = FMath::Max(0.f, NewScale); }

    UFUNCTION(BlueprintCallable, Category = "Time")
    void SetTimeOfDayNormalized(float In01) { CurrentSeconds = FMath::Clamp(In01, 0.f, 1.f) * SecondsPerDay; }

    UFUNCTION(BlueprintPure, Category = "Time")
    float GetTimeOfDayNormalized() const { return CurrentSeconds / SecondsPerDay; }

    UFUNCTION(BlueprintPure, Category = "Time")
    float GetTimeOfDayHours() const { return GetTimeOfDayNormalized() * 24.f; }

public:
    UPROPERTY(EditAnywhere, Category = "Time")
    float DayLengthRealSeconds = 600.f;

    UPROPERTY(EditAnywhere, Category = "Time")
    float StartTimeOfDayHours = 9.f;

    UPROPERTY(EditInstanceOnly, Category = "Time|Lighting")
    TSoftObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(EditAnywhere, Category = "Time|Lighting")
    float SunPitchAmplitude = 60.f;

    UPROPERTY(EditAnywhere, Category = "Time|Lighting")
    float SunYawDegreesPerDay = 360.f;

    UPROPERTY(BlueprintAssignable, Category = "Time")
    FOnTimeChanged OnTimeChanged;

protected:
	virtual void BeginPlay() override;

private:
    static constexpr float SecondsPerDay = 24.f * 3600.f;
    float CurrentSeconds = 0.f;
    float TimeScale = 1.f;
    TWeakObjectPtr<UTemperatureManager> TemperatureManager;

    void UpdateSun();
};
