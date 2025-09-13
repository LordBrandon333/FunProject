#include "StatRegistry.h"

bool UStatRegistry::GetDefaults(const FGameplayTag& Tag, FStatDefaults& OutDefaults) const
{
    if (const FStatDefaults* Found = Stats.Find(Tag))
    {
        OutDefaults = *Found;
        return true;
    }
    return false;
}
