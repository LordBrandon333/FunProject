// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractTestActor.h"

#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerCharacter.h"
#include "NeedComponent.h"
#include "StatusEffectsComponent.h"

// Sets default values
AInteractTestActor::AInteractTestActor()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
	Mesh->SetRenderCustomDepth(false);
}

void AInteractTestActor::BeginPlay()
{
	Super::BeginPlay();
}

void AInteractTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AInteractTestActor::Interact_Implementation(APlayerCharacter* Interactor)
{
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Interact: TestActor"));
	UGameplayStatics::ApplyDamage(Interactor, 20.f, nullptr, this, nullptr);

	if (UHungerComponent* Hunger = Interactor->FindComponentByClass<UHungerComponent>())
	{
		Hunger->Add(25.f);
	}
	if (UThirstComponent* Thirst = Interactor->FindComponentByClass<UThirstComponent>())
	{
		Thirst->Add(25.f);
	}

	if (UStatusEffectsComponent* Effects = Interactor->FindComponentByClass<UStatusEffectsComponent>())
	{
		Effects->ApplyEffect(EffectToApply, EffectSourceId);
	}
}

void AInteractTestActor::OnFocus_Implementation(APlayerCharacter* Interactor)
{
	if (Mesh) Mesh->SetRenderCustomDepth(true);
}

void AInteractTestActor::OnEndFocus_Implementation(APlayerCharacter* Interactor)
{
	if (Mesh) Mesh->SetRenderCustomDepth(false);
}

