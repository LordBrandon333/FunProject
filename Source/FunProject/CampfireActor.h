// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HeatSourceInterface.h"
#include "CampfireActor.generated.h"

UCLASS()
class FUNPROJECT_API ACampfireActor : public AActor, public IHeatSourceInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACampfireActor();

	UPROPERTY(EditAnywhere, Category = "Heat")
	float PeakHeatC = 20.f;

	UPROPERTY(EditAnywhere, Category = "Heat")
	float Radius = 500.f;

	virtual float GetHeatContributionAt_Implementation(const FVector& WorldLocation) const override
	{
		const float d = FVector::Distance(GetActorLocation(), WorldLocation);
		if (d >= Radius) return 0.f;
		const float alpha = 1.f - (d / Radius);
		return PeakHeatC * (alpha * alpha); // quadratischer Falloff
	}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
