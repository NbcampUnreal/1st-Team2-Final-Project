// Fill out your copyright notice in the Description page of Project Settings.


#include "UnderwaterAnimInstance.h"

#include "UnderwaterCharacter.h"
#include "KismetAnimationLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

UUnderwaterAnimInstance::UUnderwaterAnimInstance()
{
	ShouldMoveThresholdSpeed = 3.0f;

	EnvState = EEnvState::Underwater;
	CharacterState = ECharacterState::Normal;

	bIsInAir = false;
	
	Speed = 0.0f;
	ForwardSpeed = 0.0f;
	RightSpeed = 0.0f;
	UpSpeed = 0.0f;
	Direction = 0.0f;
	
	bIsStrafing = false;
	ModifyPitch = 0.0f;
	ModifyYaw = 0.0f;
	LeanThresholdSpeed = 350.0f;
	MaxPitchAngle = 10.0f;
	MaxYawAngle = 80.0f;
	ForwardDirectionTolerance = 5.0f;

	bShouldLean = false;
	
	bShouldMove = false;
	bIsGroggy = false;
	bIsDead = false;
}

void UUnderwaterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	UnderwaterCharacter = Cast<AUnderwaterCharacter>(TryGetPawnOwner());
	if (UnderwaterCharacter.IsValid())
	{
		CharacterMovement = UnderwaterCharacter->GetCharacterMovement();
	}
}

void UUnderwaterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!UnderwaterCharacter.IsValid() || !CharacterMovement.IsValid())
	{
		return;
	}
	
	UpdateVariables();

	UpdateLeanAngle();
}

void UUnderwaterAnimInstance::UpdateVariables()
{
	EnvState = UnderwaterCharacter->GetEnvState();
	CharacterState = UnderwaterCharacter->GetCharacterState();
	LocomotionMode = UnderwaterCharacter->GetLocomotionMode();
	bIsInAir = CharacterMovement->IsFalling();
	bIsGroggy = CharacterState == ECharacterState::Groggy;
	bIsDead = CharacterState == ECharacterState::Death;
	
	const FVector Velocity = UnderwaterCharacter->GetVelocity();
	Speed = Velocity.Size();

	// 최소 속도 이상이고 가속도가 0 이상일 경우에 이동 모션을 출력한다.
	// 가속도가 0일 경우고 속도가 0 이상일 경우는 밀리고 있는 상황이다.
	const FVector Accel = CharacterMovement->GetCurrentAcceleration();
	bShouldMove = Speed > ShouldMoveThresholdSpeed && !Accel.IsNearlyZero();

	ForwardSpeed = FVector::DotProduct(UnderwaterCharacter->GetActorForwardVector(), Velocity);
	RightSpeed = FVector::DotProduct(UnderwaterCharacter->GetActorRightVector(), Velocity);
	UpSpeed = FVector::DotProduct(UnderwaterCharacter->GetActorUpVector(), Velocity);

	// Actor는 Z축 기준으로만 회전하기 때문에 Rotation으로만 계산해도 현재로서는 문제 없다.

	// 2D 기준으로 Velocity의 방향을 계산한다.
	// 수직, 수평 이동일 경우는 90도가 되어버리므로 0(Forward)으로 설정한다.
	Direction = Velocity.Size2D() > KINDA_SMALL_NUMBER ? 
		UKismetAnimationLibrary::CalculateDirection(Velocity, UnderwaterCharacter->GetActorRotation())
		: 0.0f;
}

void UUnderwaterAnimInstance::UpdateLeanAngle()
{
	// 1. Lean 대상
	// - LastInputVector : 캐릭터가 이동할 때 입력한 방향 벡터
	// - Velocity : 캐릭터가 실제로 이동하는 방향 벡터
	// 현재는 Velocity를 기준으로 한다. 벽을 향해 달릴 경우 달리지 않는다.
	// 2. Lean Threshold Speed
	// - 캐릭터가 이동할 때 속도가 Lean Threshold Speed 이상일 경우에만 Lean을 한다
	// - 캐릭터가 이동할 경우 일정 속도 이상일 경우에만 앞으로 기우는 모션이기 때문이다.
	
	float TargetAngle = 0.0f;
	float InterpSpeed = 5.0f;

	const FVector Velocity = UnderwaterCharacter->GetVelocity();
	
	// -90, 90 뒤를 Move Backward로 판단한다.
	// 만약에 좌우 이동에서 판단을 하고 싶으면 추가적으로 약간의 오차를 두고 판단을 해야 한다.
	const bool bMoveForward = Direction > -90 + ForwardDirectionTolerance && Direction < 90 - ForwardDirectionTolerance;

	// Model 기준 Pitch는 Yaw이다.
	// Front(-45, 0, 45) 방향으로 이동할 때만 Lean을 한다. 그 외는 Strafing 이동이기 때문.
	// 수직, 하강은 Roll을 하면 안 된다.
	if (Speed > LeanThresholdSpeed && bMoveForward)
	{
		// UE_LOG(LogTemp,Display,TEXT("ForwardLean"))
		TargetAngle = FMath::Clamp(Direction, -MaxPitchAngle, MaxPitchAngle);
		InterpSpeed = 5.0f;
	}
	ModifyPitch = FMath::FInterpTo(ModifyPitch, TargetAngle, GetWorld()->GetDeltaSeconds(), InterpSpeed);

	// UE_LOG(LogTemp,Display,TEXT("Direction : %f"), Direction);
	// UE_LOG(LogTemp,Display, TEXT("ModifyPitch : %f"), ModifyPitch);

	// Model 기준 Roll은 Pitch이다.
	float TargetRoll = 0.0f;
	
	//  앞(-45, 0, 45)으로 이동할 경우에만 Lean을 한다.
	// 수직, 하강 이동 중도 Forward로 취급해야 한다.
	if (Speed > LeanThresholdSpeed && bMoveForward)
	{
		FRotator VelocityRotator = Velocity.ToOrientationRotator();
		TargetRoll = VelocityRotator.Pitch;
		TargetRoll = FMath::Clamp(TargetRoll, -MaxYawAngle, MaxYawAngle);
		TargetRoll *= -1.0f;
	}

	ModifyYaw = FMath::FInterpTo(ModifyYaw, TargetRoll, GetWorld()->GetDeltaSeconds(), InterpSpeed);
	// UE_LOG(LogTemp,Display,TEXT("ModifyRoll : %f"), ModifyRoll);
}
