// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"

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
	}
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
		StopRegen();
		return;
	}

	const float Old = Health;
	Health = FMath::Clamp(Health + RegenPerSecond * RegenTickInterval, 0.f, MaxHealth);
	BroadcastHealthChanged(Old, Health, Health - Old, GetOwner());

	if (Health >= MaxHealth) StopRegen();
}

void UHealthComponent::BroadcastHealthChanged(float Old, float New, float Delta, AActor* InstigatorActor)
{
	OnHealthChanged.Broadcast(this, Old, New, Delta, InstigatorActor);
}

void UHealthComponent::HandleAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.f || IsDead()) return;

	const float Scaled = Damage * FMath::Max(0.f, DamageMultiplier);
	const float Old = Health;
	Health = FMath::Clamp(Health - Scaled, 0.f, MaxHealth);

	BroadcastHealthChanged(Old, Health, Health - Old, DamageCauser);

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
}


