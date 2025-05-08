// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/ADPlayerController.h"

#include "EnhancedInputSubsystems.h"

void AADPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}
