#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ItemAction.generated.h"

class UItemInstance;
class UItemFragment;

UENUM(BlueprintType)
enum class EItemActionResult : uint8
{
	Failed, Performed, Consumed, NoOp
};

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced,
meta=(ToolTip="Item action (executes verbs; orchestrates fragments)."))
class UItemAction : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UItemInstance> OwnerInstance = nullptr;

	/** Editor hint: fragments this action expects on the item instance. */
	UPROPERTY(EditDefaultsOnly, Category="Requirements", meta=(AllowAbstract="false",
	ToolTip="Which fragments are required for this action to work?"))
	TArray<TSubclassOf<UItemFragment>> RequiredFragments;

#if WITH_EDITORONLY_DATA
	/** Cosmetic: compact summary for the details panel. */
	UPROPERTY(VisibleAnywhere, Category="Requirements", meta=(MultiLine="true"))
	FText RequirementsSummary;
#endif

	UFUNCTION(BlueprintNativeEvent) FText GetLabel() const;
	virtual FText GetLabel_Implementation() const { return FText::FromString(TEXT("Use")); }

	UFUNCTION(BlueprintNativeEvent)
	EItemActionResult Execute(AActor* User);
	virtual EItemActionResult Execute_Implementation(AActor* User) { return EItemActionResult::NoOp; }

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& E) override;
	
	/** Asset validation: warns if OwnerInstance is missing required fragments (editor-time previews). */
	virtual EDataValidationResult IsDataValid(TArray<FText>& Errors) const;
#endif
};