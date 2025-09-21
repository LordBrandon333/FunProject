#include "StatusEffectsComponent.h"
#include "GameFramework/Actor.h"

UStatusEffectsComponent::UStatusEffectsComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
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

bool UStatusEffectsComponent::ApplyEffect(UStatusEffectData* EffectData, FName SourceId)
{
    if (!EffectData) return false;

    int32 ExistingIdx = INDEX_NONE;
    for (int32 i = 0; i < Active.Num(); ++i)
    {
        const FActiveEffect& AE = Active[i];
        const bool bSameAsset = (AE.Data == EffectData);
        const bool bSameTag =
            (EffectData->EffectTag.IsValid() && AE.Data && AE.Data->EffectTag == EffectData->EffectTag);
        if (bSameAsset || bSameTag) { ExistingIdx = i; break; }
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

int32 UStatusEffectsComponent::RemoveEffectsByTag(FGameplayTag EffectTag)
{
    if (!EffectTag.IsValid()) return 0;

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

void UStatusEffectsComponent::GetAggregateForStat(const FGameplayTag& StatTag, float& OutAdd, float& OutMul) const
{
    OutAdd = 0.f;
    OutMul = 1.f;
    if (!StatTag.IsValid()) return;

    for (const FActiveEffect& AE : Active)
    {
        if (!AE.Data) continue;

        for (const FStatModifierRef& M : AE.Data->Modifiers)
        {
            if (M.StatTag != StatTag) continue;

            EStatOp Op = EStatOp::Add;
            float   V = 0.f;

            if (M.bOverrideOp)    Op = M.Op;
            if (M.bOverrideValue) V = M.Value;

            if ((!M.bOverrideOp || !M.bOverrideValue) && StatRegistry)
            {
                FStatDefaults Def;
                if (StatRegistry->GetDefaults(M.StatTag, Def))
                {
                    if (!M.bOverrideOp)    Op = Def.DefaultOp;
                    if (!M.bOverrideValue) V = Def.DefaultValue;
                }
            }

            if (M.bScaleByStacks)
            {
                const int32 Stacks = FMath::Max(1, AE.Stacks);
                if (Op == EStatOp::Add) V *= Stacks;
                else                    V = FMath::Pow(V, Stacks);
            }

            if (Op == EStatOp::Add) OutAdd += V;
            else                    OutMul *= V;
        }
    }
}

float UStatusEffectsComponent::ApplyToBase(const FGameplayTag& StatTag, float BaseValue) const
{
    float Add = 0.f, Mul = 1.f;
    GetAggregateForStat(StatTag, Add, Mul);
    return (BaseValue + Add) * Mul;
}

bool UStatusEffectsComponent::HasEffectTag(FGameplayTag EffectTag) const
{
    if (!EffectTag.IsValid()) return false;
    for (const FActiveEffect& AE : Active)
    {
        if (AE.Data && AE.Data->EffectTag == EffectTag) return true;
    }
    return false;
}
