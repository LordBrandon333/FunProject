// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractComponent.generated.h"

class APlayerCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FUNPROJECT_API UInteractComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Interact")
	void TryInteract();

	UPROPERTY(EditAnywhere, Category = "Interact")
	float TraceDistance = 300.f;

	UPROPERTY(EditAnywhere, Category = "Interact")
	float TraceRadius = 12.f;

	UPROPERTY(EditAnywhere, Category = "Interact")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_GameTraceChannel1;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawDebug = false;

	UFUNCTION(BlueprintPure, Category = "Interact")
	AActor* GetFocusedActor() const { return FocusedActor.Get(); }

private:
	TWeakObjectPtr<AActor> FocusedActor;
	void UpdateFocus();
};
