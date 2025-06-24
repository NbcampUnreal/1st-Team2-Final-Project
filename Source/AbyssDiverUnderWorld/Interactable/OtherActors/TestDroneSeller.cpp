// Fill out your copyright notice in the Description page of Project Settings.


#include "TestDroneSeller.h"

#include "ADDrone.h"


// Sets default values
ATestDroneSeller::ATestDroneSeller()
{
	// Set this actor to call Tick() very frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void ATestDroneSeller::Interact_Implementation(AActor* InstigatorActor)
{
	if (!HasAuthority())
	{
		return;
	}
	
	Super::Interact_Implementation(InstigatorActor);

	if (IsValid(CurrentDrone))
	{
		CurrentDrone->Activate();
	}
}

// Called when the game starts or when spawned
void ATestDroneSeller::BeginPlay()
{
	Super::BeginPlay();
	
}
