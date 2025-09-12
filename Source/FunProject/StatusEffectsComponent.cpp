// Fill out your copyright notice in the Description page of Project Settings.


#include "StatusEffectsComponent.h"

#include "GameFramework/Actor.h"

// Sets default values for this component's properties
UStatusEffectsComponent::UStatusEffectsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

bool UStatusEffectsComponent::ApplyEffect(UStatusEffectData* EffectData, FName SourceId)
{
    if (!EffectData) return false;

    int32 ExistingIdx = INDEX_NONE;
    for (int32 i = 0; i < Active.Num(); ++i)
    {
        const FActiveEffect& AE = Active[i];
        if (AE.Data == EffectData || (EffectData->EffectTag != NAME_None && AE.Data && AE.Data->EffectTag == EffectData->EffectTag))
        {
            ExistingIdx = i; break;
        }
    }

    if (ExistingIdx != INDEX_NONE)
    {
        FActiveEffect& AE = Active[ExistingIdx];

        switch (EffectData->Stacking)
        {
        case EEffectStackingPolicy::Additive:
            AE.Stacks = FMath::Clamp(AE.Stacks + 1, 1, EffectData->MaxStacks);
            AE.TimeRemaining = EffectData->Duration;
            break;

        case EEffectStackingPolicy::RefreshDuration:
        case EEffectStackingPolicy::Exclusive:
        default:
            AE.Stacks = 1;
            AE.TimeRemaining = EffectData->Duration;
            break;
        }
        AE.SourceId = SourceId;
    }
    else
    {
        FActiveEffect NewE;
        NewE.Data = EffectData;
        NewE.TimeRemaining = EffectData->Duration;
        NewE.Stacks = 1;
        NewE.SourceId = SourceId;
        Active.Add(MoveTemp(NewE));
        OnEffectApplied.Broadcast(EffectData);
        OnEffectsChanged.Broadcast();
    }

    return true;
}

int32 UStatusEffectsComponent::RemoveEffectsByTag(FName EffectTag)
{
    if (EffectTag == NAME_None) return 0;

    int32 RemovedCount = 0;
    for (int32 i = Active.Num() - 1; i >= 0; --i)
    {
        if (Active[i].Data && Active[i].Data->EffectTag == EffectTag)
        {
            UStatusEffectData* Removed = Active[i].Data;
            Active.RemoveAtSwap(i);
            ++RemovedCount;
            OnEffectRemoved.Broadcast(Removed);
        }
    }
    if (RemovedCount > 0) OnEffectsChanged.Broadcast();
    return RemovedCount;
}

int32 UStatusEffectsComponent::RemoveEffectsBySource(FName InSourceId)
{
    if (InSourceId == NAME_None) return 0;

    int32 RemovedCount = 0;
    for (int32 i = Active.Num() - 1; i >= 0; --i)
    {
        if (Active[i].SourceId == InSourceId)
        {
            UStatusEffectData* Removed = Active[i].Data;
            Active.RemoveAtSwap(i);
            ++RemovedCount;
            OnEffectRemoved.Broadcast(Removed);
        }
    }
    if (RemovedCount > 0) OnEffectsChanged.Broadcast();
    return RemovedCount;
}

void UStatusEffectsComponent::GetAggregateForStat(FName StatTag, float& OutAdd, float& OutMul) const
{
    OutAdd = 0.f;
    OutMul = 1.f;

    if (StatTag == NAME_None) return;

    for (const FActiveEffect& AE : Active)
    {
        if (!AE.Data) continue;

        for (const FStatModifier& M : AE.Data->Modifiers)
        {
            if (M.StatTag != StatTag) continue;

            const float V = M.Value * FMath::Max(1, AE.Stacks);
            if (M.Op == EStatOp::Add)
            {
                OutAdd += V;
            }
            else
            {
                OutMul *= V;
            }
        }
    }
}

float UStatusEffectsComponent::ApplyToBase(FName StatTag, float BaseValue) const
{
    float Add = 0.f, Mul = 1.f;
    GetAggregateForStat(StatTag, Add, Mul);
    return (BaseValue + Add) * Mul;
}

bool UStatusEffectsComponent::HasEffectTag(FName EffectTag) const
{
    if (EffectTag == NAME_None) return false;
    for (const FActiveEffect& AE : Active)
    {
        if (AE.Data && AE.Data->EffectTag == EffectTag) return true;
    }
    return false;
}

void UStatusEffectsComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UStatusEffectsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	TickEffects(DeltaTime);
}

void UStatusEffectsComponent::TickEffects(float DeltaTime)
{
	bool bChanged = false;

	for (int32 i = Active.Num() - 1; i >= 0; --i)
	{
		FActiveEffect& AE = Active[i];
		AE.TimeRemaining -= DeltaTime;
		if (AE.TimeRemaining <= 0.f)
		{
			UStatusEffectData* Removed = AE.Data;
			Active.RemoveAtSwap(i);
			bChanged = true;
			OnEffectRemoved.Broadcast(Removed);
		}
	}

	if (bChanged)
	{
		OnEffectsChanged.Broadcast();
	}
}



