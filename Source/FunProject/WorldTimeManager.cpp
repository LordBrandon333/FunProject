// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldTimeManager.h"
#include "Engine/DirectionalLight.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values
AWorldTimeManager::AWorldTimeManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AWorldTimeManager::BeginPlay()
{
	Super::BeginPlay();
	CurrentSeconds = FMath::Fmod(FMath::Max(0.f, StartTimeOfDayHours) * 3600.f, SecondsPerDay);
	UpdateSun();
	OnTimeChanged.Broadcast(GetTimeOfDayNormalized());
}

void AWorldTimeManager::UpdateSun()
{
	ADirectionalLight* Sun = SunLight.Get();
	if (!Sun) return;

	const float t = GetTimeOfDayNormalized();
	const float yaw = t * SunYawDegreesPerDay;

	const float pitch = -90.f + SunPitchAmplitude * FMath::Sin(2.f * PI * t);

	const FRotator R(pitch, yaw, 0.f);
	Sun->SetActorRotation(R);
}

// Called every frame
void AWorldTimeManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float GameDayScale = (DayLengthRealSeconds > KINDA_SMALL_NUMBER) ? (SecondsPerDay / DayLengthRealSeconds) : SecondsPerDay;

	CurrentSeconds = FMath::Fmod(CurrentSeconds + DeltaSeconds * GameDayScale * TimeScale, SecondsPerDay);

	UpdateSun();
	OnTimeChanged.Broadcast(GetTimeOfDayNormalized());
}

