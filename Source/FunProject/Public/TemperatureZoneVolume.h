// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Volume.h"
#include "TemperatureZoneVolume.generated.h"

USTRUCT(BlueprintType)
struct FTemperatureZoneModifiers
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient")
	float AmbientAddC = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient")
	float AmbientMul = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient")
	float DiurnalScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heat")
	float RadiantScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	float WindScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool bIsIndoors = false;
};


UCLASS()
class FUNPROJECT_API ATemperatureZoneVolume : public AVolume
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature Zone")
	int32 Priority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature Zone")
	FTemperatureZoneModifiers Modifiers;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	bool ContainsPoint(const FVector& WorldLocation) const;
};
