// Fill out your copyright notice in the Description page of Project Settings.


#include "NeedComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values for this component's properties
UNeedComponent::UNeedComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UNeedComponent::BeginPlay()
{
	Super::BeginPlay();

	Current = FMath::Clamp(Current, 0.f, Max);
	bIsInCritical = (Current <= CriticalThreshold);
	StartNeedTimer();
}

void UNeedComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopNeedTimer();
	Super::EndPlay(EndPlayReason);
}

void UNeedComponent::StartNeedTimer()
{
	if (TickInterval > 0.f)
	{
		GetWorld()->GetTimerManager().SetTimer(TickHandle, this, &UNeedComponent::NeedTick, TickInterval, true);
	}
}

void UNeedComponent::StopNeedTimer()
{
	GetWorld()->GetTimerManager().ClearTimer(TickHandle);
}

void UNeedComponent::NeedTick()
{
	if (bPaused) return;

	const float Old = Current;

	float Delta = 0.f;
	if (DecayPerSecond > 0.f)
	{
		Delta -= DecayPerSecond * TickInterval;
	}
	if (RegenPerSecond > 0.f && Current < Max)
	{
		Delta += RegenPerSecond * TickInterval;
	}

	if (!FMath::IsNearlyZero(Delta))
	{
		Current = FMath::Clamp(Old + Delta, 0.f, Max);
		OnNeedChanged.Broadcast(this, Old, Current, Current - Old);
		HandleThresholds(Old, Current);

		if (bDebugPrint)
		{
			UKismetSystemLibrary::PrintString(this,
				FString::Printf(TEXT("%s: %.1f/%.0f (Δ=%.2f)"), *StatTag.ToString(), Current, Max, Current - Old),
				true, false, FLinearColor::Yellow, 1.f);
		}
	}
}

void UNeedComponent::HandleThresholds(float OldValue, float NewValue)
{
	const bool bWasCritical = (OldValue <= CriticalThreshold);
	const bool bNowCritical = (NewValue <= CriticalThreshold);

	if (!bWasCritical && bNowCritical)
	{
		bIsInCritical = true;
		OnCritical.Broadcast(this);
	}
	else if (bWasCritical && !bNowCritical)
	{
		bIsInCritical = false;
		OnRecovered.Broadcast(this);
	}
}

void UNeedComponent::Add(float Amount)
{
	if (Amount <= 0.f) return;
	const float Old = Current;
	Current = FMath::Clamp(Current + Amount, 0.f, Max);
	OnNeedChanged.Broadcast(this, Old, Current, Current - Old);
	HandleThresholds(Old, Current);
}


void UNeedComponent::Consume(float Amount)
{
	if (Amount <= 0.f) return;
	const float Old = Current;
	Current = FMath::Clamp(Current - Amount, 0.f, Max);
	OnNeedChanged.Broadcast(this, Old, Current, Current - Old);
	HandleThresholds(Old, Current);
}

void UNeedComponent::SetMax(float NewMax, bool bClampCurrent, bool bKeepPercent)
{
	NewMax = FMath::Max(1.f, NewMax);
	const float Old = Current;


	if (bKeepPercent)
	{
		const float Pct = GetNormalized();
		Max = NewMax;
		Current = FMath::Clamp(Pct * Max, 0.f, Max);
	}
	else
	{
		Max = NewMax;
		if (bClampCurrent)
		{
			Current = FMath::Clamp(Current, 0.f, Max);
		}
	}


	if (!FMath::IsNearlyEqual(Old, Current))
	{
		OnNeedChanged.Broadcast(this, Old, Current, Current - Old);
		HandleThresholds(Old, Current);
	}
}

// === Derived Defaults ===
UHungerComponent::UHungerComponent()
{
	StatTag = TEXT("Hunger");
	Max = 100.f;
	Current = 100.f;
	TickInterval = 1.f;
	DecayPerSecond = 0.02f;
	RegenPerSecond = 0.f;
	CriticalThreshold = 15.f;
}

UThirstComponent::UThirstComponent()
{
	StatTag = TEXT("Thirst");
	Max = 100.f;
	Current = 100.f;
	TickInterval = 1.f;
	DecayPerSecond = 0.035f;
	RegenPerSecond = 0.f;
	CriticalThreshold = 20.f;
}

