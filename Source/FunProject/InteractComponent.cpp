// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractComponent.h"

#include "Interactable.h"
#include "PlayerCharacter.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values for this component's properties
UInteractComponent::UInteractComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UInteractComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UInteractComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateFocus();
}

void UInteractComponent::TryInteract()
{
	AActor* Owner = GetOwner();
	AActor* Target = FocusedActor.Get();
	if (!Owner || !Target) return;

	if (!Target->GetClass()->ImplementsInterface(UInteractable::StaticClass())) return;

	APlayerCharacter* PC = Cast<APlayerCharacter>(Owner);
	if (IInteractable::Execute_CanInteract(Target, PC))
	{
		IInteractable::Execute_Interact(Target, PC);
	}
}

void UInteractComponent::UpdateFocus()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	FVector EyeLoc; FRotator EyeRot;
	if (ACharacter* Char = Cast<ACharacter>(Owner)) Char->GetActorEyesViewPoint(EyeLoc, EyeRot);
	else Owner->GetActorEyesViewPoint(EyeLoc, EyeRot);

	const FVector End = EyeLoc + EyeRot.Vector() * TraceDistance;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(InteractTrace), false, Owner);

	const bool bHit = GetWorld()->SweepSingleByChannel(Hit, EyeLoc, End, FQuat::Identity, TraceChannel, FCollisionShape::MakeSphere(TraceRadius), Params);

	AActor* NewFocus = bHit ? Hit.GetActor() : nullptr;
	if (bDrawDebug) GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Green, UKismetSystemLibrary::GetDisplayName(NewFocus));
	AActor* OldFocus = FocusedActor.Get();

	if (NewFocus == OldFocus)
	{
		if (bDrawDebug) 
		{
			if (!bHit) DrawDebugLine(GetWorld(), EyeLoc, End, FColor::Red, false, 0.f, 0, 0.1f);
			else DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 8.f, 12, NewFocus->GetClass()->ImplementsInterface(UInteractable::StaticClass()) ? FColor::Green : FColor::Yellow, false, 0.f);
		}
		return;
	}

	if (OldFocus && OldFocus->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
	{
		IInteractable::Execute_OnEndFocus(OldFocus, Cast<APlayerCharacter>(Owner));
	}

	FocusedActor = NewFocus;

	if (NewFocus && NewFocus->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
	{
		IInteractable::Execute_OnFocus(NewFocus, Cast<APlayerCharacter>(Owner));
	}
}

