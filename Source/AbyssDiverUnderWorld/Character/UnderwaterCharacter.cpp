// Fill out your copyright notice in the Description page of Project Settings.


#include "UnderwaterCharacter.h"

#include "AbyssDiverUnderWorld.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AUnderwaterCharacter::AUnderwaterCharacter()
{
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
}

void AUnderwaterCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AUnderwaterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInput->BindAction(
				MoveAction, 
				ETriggerEvent::Triggered, 
				this, 
				&AUnderwaterCharacter::Move
			);
			EnhancedInput->BindAction(
				MoveAction, 
				ETriggerEvent::Started, 
				this, 
				&AUnderwaterCharacter::Move
			);
		}

		if (LookAction)
		{
			EnhancedInput->BindAction(
				LookAction,
				ETriggerEvent::Triggered,
				this,
				&AUnderwaterCharacter::Look
			);
		}

		if (FireAction)
		{
			EnhancedInput->BindAction(
				FireAction,
				ETriggerEvent::Started,
				this,
				&AUnderwaterCharacter::Fire
			);
		}

		if (AimAction)
		{
			EnhancedInput->BindAction(
				AimAction,
				ETriggerEvent::Started,
				this,
				&AUnderwaterCharacter::Aim
			);
		}

		if (InteractionAction)
		{
			EnhancedInput->BindAction(
				InteractionAction,
				ETriggerEvent::Started,
				this,
				&AUnderwaterCharacter::Interaction
			);
		}

		if (LightAction)
		{
			EnhancedInput->BindAction(
				LightAction,
				ETriggerEvent::Started,
				this,
				&AUnderwaterCharacter::Light
			);
		}

		if (RadarAction)
		{
			EnhancedInput->BindAction(
				RadarAction,
				ETriggerEvent::Started,
				this,
				&AUnderwaterCharacter::Radar
			);
		}
	}
	else
	{
		UE_LOG(AbyssDiver, Error, TEXT("Failed to find an Enhanced Input Component."))
	}
}

void AUnderwaterCharacter::Move(const FInputActionValue& InputActionValue)
{

}

void AUnderwaterCharacter::Look(const FInputActionValue& InputActionValue)
{
}

void AUnderwaterCharacter::Fire(const FInputActionValue& InputActionValue)
{
}

void AUnderwaterCharacter::Aim(const FInputActionValue& InputActionValue)
{
}

void AUnderwaterCharacter::Interaction(const FInputActionValue& InputActionValue)
{
}

void AUnderwaterCharacter::Light(const FInputActionValue& InputActionValue)
{
}

void AUnderwaterCharacter::Radar(const FInputActionValue& InputActionValue)
{
}

