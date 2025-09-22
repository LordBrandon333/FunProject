#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ItemAction.generated.h"

class UItemInstance;

UENUM(BlueprintType)
enum class EItemActionResult : uint8
{
	Failed, Performed, Consumed, NoOp
};

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class UItemAction : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UItemInstance> OwnerInstance = nullptr;

	UFUNCTION(BlueprintNativeEvent) FText GetLabel() const;
	virtual FText GetLabel_Implementation() const { return FText::FromString(TEXT("Use")); }

	UFUNCTION(BlueprintNativeEvent)
	EItemActionResult Execute(AActor* User);
	virtual EItemActionResult Execute_Implementation(AActor* User) { return EItemActionResult::NoOp; }
};