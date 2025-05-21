// Fill out your copyright notice in the Description page of Project Settings.


#include "UnderwaterAnimInstance.h"

#include "UnderwaterCharacter.h"
#include "KismetAnimationLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

UUnderwaterAnimInstance::UUnderwaterAnimInstance()
{
	ShouldMoveThresholdSpeed = 3.0f;

	EnvState = EEnvState::Underwater;
	
	Speed = 0.0f;
	ForwardSpeed = 0.0f;
	RightSpeed = 0.0f;
	UpSpeed = 0.0f;
	Direction = 0.0f;
	
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
}

void UUnderwaterAnimInstance::UpdateVariables()
{
	EnvState = UnderwaterCharacter->GetEnvState();
	CharacterState = UnderwaterCharacter->GetCharacterState();
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
	Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, UnderwaterCharacter->GetActorRotation());
}
