// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"
#include "StatusEffectsComponent.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::Heal(float Amount)
{
	if (Amount <= 0.f || IsDead()) return;

	const float Old = Health;
	Health = FMath::Clamp(Health + Amount, 0.f, MaxHealth);
	BroadcastHealthChanged(Old, Health, Health - Old, GetOwner());
}

void UHealthComponent::Kill()
{
	if (IsDead()) return;
	const float Old = Health;
	Health = 0.f;
	BroadcastHealthChanged(Old, Health, Health - Old, GetOwner());
	StopRegen();
	OnDeath.Broadcast(this, GetOwner());
}

void UHealthComponent::SetMaxHealth(float NewMax, bool bClampCurrent, bool bKeepPercent)
{
	NewMax = FMath::Max(1.f, NewMax);
	const float OldMax = MaxHealth;
	const float Old = Health;

	if (bKeepPercent)
	{
		const float Pct = GetHealthNormalized();
		MaxHealth = NewMax;
		Health = FMath::Clamp(Pct * MaxHealth, 0.f, MaxHealth);
	}
	else
	{
		MaxHealth = NewMax;
		if (bClampCurrent)
		{
			Health = FMath::Clamp(Health, 0.f, MaxHealth);
		}
	}

	if (!FMath::IsNearlyEqual(Old, Health))
	{
		BroadcastHealthChanged(Old, Health, Health - Old, GetOwner());
	}
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = FMath::Clamp(MaxHealth, 1.f, MaxHealth);

	if (AActor* Owner = GetOwner())
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleAnyDamage);

		if (!Effects.IsValid()) Effects = Owner->FindComponentByClass<UStatusEffectsComponent>();
		if (Effects.IsValid()) Effects->OnEffectsChanged.AddDynamic(this, &UHealthComponent::OnEffectsChanged);
	}

	if (!DamageReceivedTag.IsValid()) DamageReceivedTag = FGameplayTag::RequestGameplayTag(TEXT("Stat.Damage.Received"), false);
	if (!HealthRegenTag.IsValid()) HealthRegenTag = FGameplayTag::RequestGameplayTag(TEXT("Stat.Health.Regen"), false);

	NextAllowedPositiveRegenTime = GetWorld()->GetTimeSeconds() + RegenDelay;

	RefreshRegenTimer();
}

void UHealthComponent::StartRegen()
{
	if (!bAutoRegenEnabled || IsDead() || Health >= MaxHealth) return;
	GetWorld()->GetTimerManager().SetTimer(RegenTickHandle, this, &UHealthComponent::RegenTick, RegenTickInterval, true);
}

void UHealthComponent::StopRegen()
{
	GetWorld()->GetTimerManager().ClearTimer(RegenTickHandle);
	GetWorld()->GetTimerManager().ClearTimer(RegenDelayHandle);
}

void UHealthComponent::RegenTick()
{
	if (IsDead()) 
	{ 
		RefreshRegenTimer(); 
		return; 
	}

	const float Now = GetWorld()->GetTimeSeconds();
	const float Rate = GetEffectiveHealthRegenRate(); 

	if (FMath::IsNearlyZero(Rate))
	{
		RefreshRegenTimer();
		return;
	}

	if (Rate > 0.f)
	{
		if (Health >= MaxHealth || Now < NextAllowedPositiveRegenTime) return;
	}

	const float DeltaHP = Rate * RegenTickInterval;
	const float Old = Health;

	Health = FMath::Clamp(Health + DeltaHP, 0.f, MaxHealth);
	BroadcastHealthChanged(Old, Health, Health - Old, GetOwner());

	if (Old > 0.f && Health <= 0.f)
	{
		GetWorld()->GetTimerManager().ClearTimer(RegenTickHandle);
		OnDeath.Broadcast(this, GetOwner());
		return;
	}

	RefreshRegenTimer();
}

float UHealthComponent::GetEffectiveHealthRegenRate() const
{
	float Rate = RegenPerSecond;
	if (Effects.IsValid() && HealthRegenTag.IsValid())
	{
		float Add = 0.f, Mul = 1.f;
		Effects->GetAggregateForStat(HealthRegenTag, Add, Mul);
		Rate = (Rate + Add) * Mul;
	}
	return Rate;
}

void UHealthComponent::RefreshRegenTimer()
{
	const float Interval = RegenTickInterval;
	const float EffectiveHealthRegenRate = GetEffectiveHealthRegenRate();
	const bool bShouldTick = !IsDead() && (EffectiveHealthRegenRate != 0.f) && !(EffectiveHealthRegenRate >= 0.f && Health >= MaxHealth);

	FTimerManager& TM = GetWorld()->GetTimerManager();

	const bool bTimerActive = TM.IsTimerActive(RegenTickHandle);

	if (bShouldTick && !bTimerActive)
	{
		TM.SetTimer(RegenTickHandle, this, &UHealthComponent::RegenTick, Interval, true);
	}
	else if (!bShouldTick && bTimerActive)
	{
		TM.ClearTimer(RegenTickHandle);
	}
}

void UHealthComponent::OnEffectsChanged()
{
	RefreshRegenTimer();
}

void UHealthComponent::BroadcastHealthChanged(float Old, float New, float Delta, AActor* InstigatorActor)
{
	OnHealthChanged.Broadcast(this, Old, New, Delta, InstigatorActor);
}

void UHealthComponent::HandleAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.f || IsDead()) return;

	float Scale = DamageMultiplier;

	if (Effects.IsValid() && DamageReceivedTag.IsValid())
	{
		float Add = 0.f, Mul = 1.f;
		Effects->GetAggregateForStat(DamageReceivedTag, Add, Mul);
		Scale = (Scale + Add)* Mul;
	}

	const float Old = Health;
	const float Scaled = Damage * FMath::Max(0.f, Scale);
	Health = FMath::Clamp(Health - Scaled, 0.f, MaxHealth);
	BroadcastHealthChanged(Old, Health, Health - Old, DamageCauser);

	NextAllowedPositiveRegenTime = GetWorld()->GetTimeSeconds() + RegenDelay;

	/*
	if (bAutoRegenEnabled)
	{
		StopRegen();
		if (!IsDead())
		{
			GetWorld()->GetTimerManager().SetTimer(RegenDelayHandle, this, &UHealthComponent::StartRegen, RegenDelay, false);
		}
	}
	
	if (IsDead())
	{
		StopRegen();
		OnDeath.Broadcast(this, DamagedActor);
	}
	*/

	if (IsDead())
	{
		GetWorld()->GetTimerManager().ClearTimer(RegenTickHandle);
		OnDeath.Broadcast(this, DamagedActor);
		return;
	}

	RefreshRegenTimer();
}


