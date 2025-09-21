// Fill out your copyright notice in the Description page of Project Settings.


#include "CampfireActor.h"

// Sets default values
ACampfireActor::ACampfireActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACampfireActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACampfireActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

