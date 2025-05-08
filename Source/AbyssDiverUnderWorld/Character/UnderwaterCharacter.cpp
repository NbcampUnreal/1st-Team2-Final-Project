// Fill out your copyright notice in the Description page of Project Settings.


#include "UnderwaterCharacter.h"

#include "AbyssDiverUnderWorld.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PhysicsVolume.h"

AUnderwaterCharacter::AUnderwaterCharacter()
{
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Swimming);
		Movement->MaxSwimSpeed = 400.0f;
	}
	
	// Temp: Player Mesh를 완전히 숨긴다.
	// 추후 Player Mesh를 추가해서 보여주거나 일부분만 숨길 수 있는 방법에 대해서 찾아야 한다.
	GetMesh()->SetOwnerNoSee(true);	
}

void AUnderwaterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		// 현재 Physics Volume을 Water로 설정한다.
		// 현재 Physics Volume이 Water가 아닐 경우 World의 Default Volume을 반환한다.
		// Default Volume을 Water를 설정해서 Swim Mode를 사용할 수 있도록 한다.
		Movement->GetPhysicsVolume()->bWaterVolume = true;
	}
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
	// 캐릭터의 XYZ 축을 기준으로 입력을 받는다.
	const FVector MoveInput = InputActionValue.Get<FVector>();
	
	// Forward : Camera Forward with pitch
	const FRotator ControlRotation = GetControlRotation();
	const FVector ForwardVector = FRotationMatrix(ControlRotation).GetUnitAxis(EAxis::X);

	// Right : Camera Right without pitch
	const FRotator XYRotation = FRotator(0.f, ControlRotation.Yaw, 0.f);
	const FVector RightVector = FRotationMatrix(XYRotation).GetUnitAxis(EAxis::Y);

	if (!FMath::IsNearlyZero(MoveInput.X)) 
	{
		AddMovementInput(ForwardVector, MoveInput.X);
	}

	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		AddMovementInput(RightVector, MoveInput.Y);
	}
}

void AUnderwaterCharacter::Look(const FInputActionValue& InputActionValue)
{
	FVector2d LookInput = InputActionValue.Get<FVector2d>();

	// Y축은 반전되어서 들어오므로 그대로 적용한다.
	AddControllerYawInput(LookInput.X);
	AddControllerPitchInput(LookInput.Y);
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

