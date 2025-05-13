// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/ADPlayerState.h"
#include "Inventory/ADInventoryComponent.h"
#include "AbyssDiverUnderWorld.h"

AADPlayerState::AADPlayerState()
{
	InventoryComp = CreateDefaultSubobject<UADInventoryComponent>(TEXT("InventoryComp"));
}



void AADPlayerState::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetPlayerController();
	if (PC && PC->IsLocalController() && HasAuthority())
	{

		InventoryComp->InventoryInitialize();
		LOGVN(Error, TEXT("Inventory Initializded"));
	}

}

void AADPlayerState::PostNetInit()
{
	Super::PostNetInit();

	APlayerController* PC = GetPlayerController();
	if (PC && PC->IsLocalController())
	{

		InventoryComp->InventoryInitialize();
		LOGVN(Error, TEXT("Inventory Initializded"));
	}
}
