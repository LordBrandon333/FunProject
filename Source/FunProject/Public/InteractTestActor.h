// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "StatusEffectData.h"
#include "InteractTestActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class FUNPROJECT_API AInteractTestActor : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	AInteractTestActor();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	//IInteractable Overrides

	virtual bool CanInteract_Implementation(APlayerCharacter* Interactor) override { return true; }
	virtual void Interact_Implementation(APlayerCharacter* Interactor) override;
	virtual void OnFocus_Implementation(APlayerCharacter* Interactor) override;
	virtual void OnEndFocus_Implementation(APlayerCharacter* Interactor) override;

	UPROPERTY(EditDefaultsOnly, Category = "Test")
	UStatusEffectData* EffectToApply = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Test")
	FName EffectSourceId = TEXT("Example");
};
