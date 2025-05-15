// Fill out your copyright notice in the Description page of Project Settings.


#include "UnderwaterCharacter.h"

#include "AbyssDiverUnderWorld.h"
#include "EnhancedInputComponent.h"
#include "PlayerComponent/OxygenComponent.h"
#include "PlayerComponent/StaminaComponent.h"
#include "StatComponent.h"
#include "AbyssDiverUnderWorld/Interactable/Item/Component/ADInteractionComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PhysicsVolume.h"
#include "GameFramework/SpringArmComponent.h"
#include "Shops/ShopInteractionComponent.h"

AUnderwaterCharacter::AUnderwaterCharacter()
{
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	StatComponent->Initialize(1000, 1000, 400.0f, 10);
	
	bIsCaptured = false;
	CaptureFadeTime = 0.5f;

	BloodEmitNoiseRadius = 1.0f;
	
	StatComponent->MoveSpeed = 400.0f;
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Swimming);
		Movement->MaxSwimSpeed = StatComponent->MoveSpeed;
		Movement->BrakingDecelerationSwimming = 500.0f;
		Movement->GravityScale = 0.0f;
	}
	SprintSpeed = StatComponent->MoveSpeed * 1.75f;

	// To-Do
	// 외부에 보여지는 Mesh와 1인칭 Mesh를 다르게 구현
	GetMesh()->SetOwnerNoSee(true);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetCapsuleComponent());
	CameraBoom->TargetArmLength = 300.f;
	CameraBoom->bUsePawnControlRotation = false;

	ThirdPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	ThirdPersonCameraComponent->bUsePawnControlRotation = false;

	bUseDebugCamera = false;
	ThirdPersonCameraComponent->SetActive(false);

	// To-Do
	// 외부에 보여지는 Mesh와 1인칭 Mesh를 다르게 구현
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->bCastHiddenShadow = true;
	
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->CastShadow = false;
	Mesh1P->bCastDynamicShadow = false;
	
	OxygenComponent = CreateDefaultSubobject<UOxygenComponent>(TEXT("OxygenComponent"));
	StaminaComponent = CreateDefaultSubobject<UStaminaComponent>(TEXT("StaminaComponent"));

	InteractionComponent = CreateDefaultSubobject<UADInteractionComponent>(TEXT("InteractionComponent"));
	ShopInteractionComponent = CreateDefaultSubobject<UShopInteractionComponent>(TEXT("ShopInteractionComponent"));

	CharacterState = ECharacterState::Underwater;
}

void AUnderwaterCharacter::BeginPlay()
{
	Super::BeginPlay();

	SetCharacterState(CharacterState);

	SetDebugCameraMode(bUseDebugCamera);

	StaminaComponent->OnSprintStateChanged.AddDynamic(this, &AUnderwaterCharacter::OnSprintStateChanged);

	NoiseEmitterComponent = NewObject<UPawnNoiseEmitterComponent>(this);
	NoiseEmitterComponent->RegisterComponent();
}

void AUnderwaterCharacter::SetCharacterState(ECharacterState State)
{
	CharacterState = State;

	switch (CharacterState)
	{
	case ECharacterState::Underwater:
		// 현재 Physics Volume을 Water로 설정한다.
		// 현재 Physics Volume이 Water가 아닐 경우 World의 Default Volume을 반환한다.
		// Default Volume을 Water를 설정해서 Swim Mode를 사용할 수 있도록 한다.
		GetCharacterMovement()->GetPhysicsVolume()->bWaterVolume = true;
		GetCharacterMovement()->SetMovementMode(MOVE_Swimming);
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->GravityScale = 0.0f;
		bUseControllerRotationYaw = true;
		break;
	case ECharacterState::Ground:
		// 지상에서는 이동 방향으로 회전을 하게 한다.
		GetCharacterMovement()->GetPhysicsVolume()->bWaterVolume = false;
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->GravityScale = 1.0f;
		bUseControllerRotationYaw = false;
		break;
	default:
		UE_LOG(AbyssDiver, Error, TEXT("Invalid Character State"));
		break;
	}
}

void AUnderwaterCharacter::StartCaptureState()
{
	if(bIsCaptured || !HasAuthority())
	{
		return;
	}

	bIsCaptured = true;
	M_StartCaptureState();
}

void AUnderwaterCharacter::StopCaptureState()
{
	// 사망 처리 시에도 StopCaptureState가 호출된다.
	// 즉, 사망이 확정되는 시점이 다르게 작동할 수 있다.
	
	if (!bIsCaptured || !HasAuthority())
	{
		return;
	}

	bIsCaptured = false;
	M_StopCaptureState();
}

void AUnderwaterCharacter::EmitBloodNoise()
{
	if (NoiseEmitterComponent)
	{
		NoiseEmitterComponent->MakeNoise(this, BloodEmitNoiseRadius, GetActorLocation());
	}
}

void AUnderwaterCharacter::M_StartCaptureState_Implementation()
{
	if (IsLocallyControlled())
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			PlayerController->SetIgnoreLookInput(true);
			PlayerController->SetIgnoreMoveInput(true);

			PlayerController->PlayerCameraManager->StartCameraFade(
				0.0f,
				1.0f,
				CaptureFadeTime,
				FLinearColor::Black,
				false,
				true
			);
		}
		// Play SFX
	}

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

void AUnderwaterCharacter::M_StopCaptureState_Implementation()
{
	if (IsLocallyControlled())
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			PlayerController->ResetIgnoreLookInput();
			PlayerController->ResetIgnoreMoveInput();

			PlayerController->PlayerCameraManager->StartCameraFade(
				1.0f,
				0.0f,
				CaptureFadeTime,
				FLinearColor::Black,
				false,
				true
			);
		}
	}
	// Play SFX

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
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
				ETriggerEvent::Started, 
				this, 
				&AUnderwaterCharacter::Move
			);
			EnhancedInput->BindAction(
				MoveAction, 
				ETriggerEvent::Triggered, 
				this, 
				&AUnderwaterCharacter::Move
			);
		}

		if (SprintAction)
		{
			EnhancedInput->BindAction(
				SprintAction,
				ETriggerEvent::Started,
				this,
				&AUnderwaterCharacter::StartSprint
			);
			EnhancedInput->BindAction(
				SprintAction,
				ETriggerEvent::Completed,
				this,
				&AUnderwaterCharacter::StopSprint
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

float AUnderwaterCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (ActualDamage > 0.0f)
	{
		EmitBloodNoise();
	}

	return ActualDamage;
}

void AUnderwaterCharacter::Move(const FInputActionValue& InputActionValue)
{
	// To-Do
	// Can Move 확인
	
	// 캐릭터의 XYZ 축을 기준으로 입력을 받는다.
	const FVector MoveInput = InputActionValue.Get<FVector>();

	if (CharacterState == ECharacterState::Ground)
	{
		MoveGround(MoveInput);
	}
	else if (CharacterState == ECharacterState::Underwater)
	{
		MoveUnderwater(MoveInput);
	}
}

void AUnderwaterCharacter::MoveUnderwater(const FVector MoveInput)
{
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

void AUnderwaterCharacter::MoveGround(FVector MoveInput)
{
	const FRotator ControllerRotator = FRotator(0.f, Controller->GetControlRotation().Yaw, 0.f);

	// World에서 Controller의 X축, Y축 방향
	const FVector ForwardVector = FRotationMatrix(ControllerRotator).GetUnitAxis(EAxis::X);
	const FVector RightVector = FRotationMatrix(ControllerRotator).GetUnitAxis(EAxis::Y);

	// 전후 이동
	if (!FMath::IsNearlyZero(MoveInput.X))
	{
		AddMovementInput(ForwardVector, MoveInput.X);
	}
	// 좌우 이동
	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		AddMovementInput(RightVector, MoveInput.Y);
	}
}

void AUnderwaterCharacter::StartSprint(const FInputActionValue& InputActionValue)
{
	StaminaComponent->RequestStartSprint();
}

void AUnderwaterCharacter::StopSprint(const FInputActionValue& InputActionValue)
{
	StaminaComponent->RequestStopSprint();
}

void AUnderwaterCharacter::OnSprintStateChanged(bool bNewSprinting)
{
	if (bNewSprinting)
	{
		GetCharacterMovement()->MaxSwimSpeed = SprintSpeed;
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxSwimSpeed = StatComponent->MoveSpeed;
		GetCharacterMovement()->MaxWalkSpeed = StatComponent->MoveSpeed;
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
	InteractionComponent->TryInteract();
}

void AUnderwaterCharacter::Light(const FInputActionValue& InputActionValue)
{
}

void AUnderwaterCharacter::Radar(const FInputActionValue& InputActionValue)
{
}

void AUnderwaterCharacter::SetDebugCameraMode(bool bDebugCameraEnable)
{
	bUseDebugCamera = bDebugCameraEnable;
	if (bUseDebugCamera)
	{
		FirstPersonCameraComponent->SetActive(false);
		ThirdPersonCameraComponent->SetActive(true);
		GetMesh()->SetOwnerNoSee(false);
	}
	else
	{
		FirstPersonCameraComponent->SetActive(true);
		ThirdPersonCameraComponent->SetActive(false);
		GetMesh()->SetOwnerNoSee(true);
	}
}

void AUnderwaterCharacter::ToggleDebugCameraMode()
{
	bUseDebugCamera = !bUseDebugCamera;
	SetDebugCameraMode(bUseDebugCamera);
}

bool AUnderwaterCharacter::IsSprinting() const
{
	return StaminaComponent->IsSprinting();
}
