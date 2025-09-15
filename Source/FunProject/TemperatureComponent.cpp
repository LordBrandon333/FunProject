// Fill out your copyright notice in the Description page of Project Settings.


#include "TemperatureComponent.h"
#include "WorldTimeManager.h"
#include "HeatSourceInterface.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HealthComponent.h"



UTemperatureComponent::UTemperatureComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UTemperatureComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AWorldTimeManager* M = Cast<AWorldTimeManager>(UGameplayStatics::GetActorOfClass(this, AWorldTimeManager::StaticClass())))
	{
		TimeMgr = M;
	}
	if (AActor* Owner = GetOwner())
	{
		Health = Owner->FindComponentByClass<UHealthComponent>();
	}

	GetWorld()->GetTimerManager().SetTimer(TickHandle, this, &UTemperatureComponent::UpdateOnce, UpdateInterval, true, 0.1f);
}

void UTemperatureComponent::UpdateOnce()
{
	const FVector Loc = GetOwner()->GetActorLocation();

	const float OldBody = BodyTempC;
	const float OldAmbient = AmbientTempC;

	AmbientTempC = ComputeAmbientAt(Loc);
	const float heatSorces = SumHeatSourcesAt(Loc);
	const float metabolic = ComputeMetabolicHeat();

	const float baseK = 0.12f;
	const float k = baseK * (1.f - Insulation) * (1.f + 1.5f * WindExposure) * (1.f + 1.0f * Wetness);

	const float target = AmbientTempC + heatSorces + metabolic;
	BodyTempC += (target - BodyTempC) * (k * UpdateInterval);

	if (!FMath::IsNearlyEqual(OldAmbient, AmbientTempC, 0.05f))
	{
		OnAmbientTempChanged.Broadcast(OldAmbient, AmbientTempC);
	}

	if (!FMath::IsNearlyEqual(OldBody, BodyTempC, 0.01f))
	{
		OnBodyTempChanged.Broadcast(OldBody, BodyTempC);
		ApplyHealthEffects(OldBody, BodyTempC);
	}
}

float UTemperatureComponent::ComputeAmbientAt(const FVector& Loc) const
{
	float ambient = 15.f;

	if (TimeMgr.IsValid())
	{
		const float t = TimeMgr->GetTimeOfDayNormalized();
		if (DiurnalCurve) ambient += DiurnalCurve->GetFloatValue(t);
		else
		{
			ambient += 8.f * FMath::Sin(2.f * PI * (t - 0.1f));
		}
	}

	ambient += ElevationLapseRateCPerMeter * (Loc.Z / 100.f);
	ambient -= 4.f * WindExposure;

	// TODO: Add Weather, Biome etc.

	return ambient;
}

float UTemperatureComponent::SumHeatSourcesAt(const FVector& Loc) const
{
	TArray<AActor*> Candidates;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UHeatSourceInterface::StaticClass(), Candidates);

	float sum = 0.f;
	for (AActor* A : Candidates)
	{
		if (!A) continue;
		const float c = IHeatSourceInterface::Execute_GetHeatContributionAt(A, Loc);
		sum += c;
	}
	return sum;
}

float UTemperatureComponent::ComputeMetabolicHeat() const
{
	const ACharacter* C = Cast<ACharacter>(GetOwner());
	if (!C) return 0.2f;

	const auto* Move = C->GetCharacterMovement();
	const float speed = Move ? C->GetVelocity().Size() : 0.f;

	return 0.2f + FMath::Clamp(speed / 600.f, 0.f, 1.f) * 0.8f;
}

void UTemperatureComponent::ApplyHealthEffects(float OldC, float NewC)
{
	if (!Health.IsValid()) return;

	if (NewC < SafeBodyTempRangeC.X)
	{
		// z.B. alle Updates kleiner „Kälteschaden“
		//Health->ApplyDamage(0.1f, GetOwner()); // TODO: scale by delta
	}
	else if (NewC > SafeBodyTempRangeC.Y)
	{
		//Health->ApplyDamage(0.1f, GetOwner());
	}
}


