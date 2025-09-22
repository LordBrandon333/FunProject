#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ItemFragment.generated.h"

class UItemInstance;

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class UItemFragment : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UItemInstance> OwnerInstance = nullptr;
	
	UFUNCTION(BlueprintNativeEvent) void InitializeFragment();
	virtual void InitializeFragment_Implementation() {}
	
	UFUNCTION(BlueprintNativeEvent) void BuildTooltip(TArray<FText>& OutLines) const;
	virtual void BuildTooltip_Implementation(TArray<FText>& OutLines) const {}
};