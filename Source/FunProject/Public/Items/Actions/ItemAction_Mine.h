#pragma once
#include "Items/ItemAction.h"
#include "ItemAction_Mine.generated.h"


UCLASS(BlueprintType, EditInlineNew)
class UItemAction_Mine : public UItemAction
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mine")
	int32 DurabilityCostPerHit = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mine")
	float Range = 250.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mine")
	FName HitBoneName;
	
	virtual FText GetLabel_Implementation() const override { return FText::FromString(TEXT("Mine")); }
	virtual EItemActionResult Execute_Implementation(AActor* User) override;
};