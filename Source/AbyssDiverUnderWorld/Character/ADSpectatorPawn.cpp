// Fill out your copyright notice in the Description page of Project Settings.


#include "ADSpectatorPawn.h"

#include "EnhancedInputComponent.h"
#include "UnderwaterCharacter.h"


AADSpectatorPawn::AADSpectatorPawn()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AADSpectatorPawn::BeginPlay()
{
	Super::BeginPlay();
}

void AADSpectatorPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (ViewNextPlayerAction)
		{
			EnhancedInput->BindAction(
				ViewNextPlayerAction,
				ETriggerEvent::Started,
				this,
				&AADSpectatorPawn::ViewNextPlayer
			);
		}

		if (ViewPrevPlayerAction)
		{
			EnhancedInput->BindAction(
				ViewPrevPlayerAction,
				ETriggerEvent::Started,
				this,
				&AADSpectatorPawn::ViewPrevPlayer
			);
		}
	}
}

void AADSpectatorPawn::ViewNextPlayer()
{
	if (APlayerController* PlayerController = GetController<APlayerController>())
	{
		PlayerController->ServerViewNextPlayer();
	}
}

void AADSpectatorPawn::ViewPrevPlayer()
{
	if (APlayerController* PlayerController = GetController<APlayerController>())
	{
		PlayerController->ServerViewPrevPlayer();
	}
}