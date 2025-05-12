// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/ADPlayerController.h"

#include "AbyssDiverUnderWorld.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "ADPlayerState.h"
#include "Inventory/ADInventoryComponent.h"

AADPlayerController::AADPlayerController()
{
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> MappingContextAsset(TEXT("/Game/_AbyssDiver/Input/IMC_Player.IMC_Player"));
	if (MappingContextAsset.Succeeded())
	{
		DefaultMappingContext = MappingContextAsset.Object;
	}
	else
	{
		UE_LOG(AbyssDiver, Warning, TEXT("Failed to load InputMappingContext"));
	}
}

void AADPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	Cast<AADPlayerState>(PlayerState)->GetInventory()->InventoryInitialize();
}
