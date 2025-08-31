// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnderwaterCharacter.h"
#include "Animation/AnimInstance.h"
#include "UnderwaterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UUnderwaterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UUnderwaterAnimInstance();
	
protected:
	
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

#pragma region Method

protected:
	
	/** 캐릭터 상태 변수 업데이트 */
	void UpdateVariables();

	/** 캐릭터 기울기 업데이트 */
	void UpdateLeanAngle();
	
#pragma endregion
	
#pragma region Variable

public:
	
	/** Strafing 적용 여부. Strafing Mode일 경우 직립 상태로만 이동한다. */
	UPROPERTY(BlueprintReadWrite)
	uint8 bIsStrafing : 1;
	
protected:
	
	/** Character 약참조 */
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<class AUnderwaterCharacter> UnderwaterCharacter;

	/** Movement 약참조 */
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<class UCharacterMovementComponent> CharacterMovement;

	/** 캐릭터의 현재 환경 상태 */
	UPROPERTY(BlueprintReadOnly)
	EEnvironmentState EnvironmentState;

	/** 캐릭터의 현재 상태. Groggy, Dead, Normal */
	UPROPERTY(BlueprintReadOnly)
	ECharacterState CharacterState;

	/** 캐릭터의 공중 상태. Fall, Jump를 구분하기 위함 */
	UPROPERTY(BlueprintReadOnly)
	ELocomotionMode LocomotionMode;
	
	/** 캐릭터가 공중에 있는지 여부, Movement Mode Falling 상태인지 여부 */
	UPROPERTY(BlueprintReadOnly)
	uint8 bIsInAir : 1;
	
	/** 캐릭터 Should Move에서 Move를 가능하게 하는 임계값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ShouldMoveThresholdSpeed;

	/** 속도 벡터의 크기 */
	UPROPERTY(BlueprintReadOnly)
	float Speed;

	/* 속도의 Forward 크기(World Space 기준 Actor Forward 방향) */
	UPROPERTY(BlueprintReadOnly)
	float ForwardSpeed;

	/** 속도의 Right 크기(World Space 기준 Actor Right 방향) */
	UPROPERTY(BlueprintReadOnly)
	float RightSpeed;
	
	/** 속도의 Up 크기(World Space 기준 Actor Up 방향) */
	UPROPERTY(BlueprintReadOnly)
	float UpSpeed;

	/** 속도 Vector와 Forward Vector의 각도 [-180, 180] */
	UPROPERTY(BlueprintReadOnly)
	float Direction;
	
	/** 단순하게 속도를 기준으로 하면 정지했을 때 속도가 줄지 않았을 경우 이동 모션이 출력이 된다. 따라서 가속도를 고려한다. */
	UPROPERTY(BlueprintReadOnly)
	uint8 bShouldMove : 1;

	/** 캐릭터가 Groggy 상태인지 여부 */
	UPROPERTY(BlueprintReadOnly)
	uint8 bIsGroggy : 1;

	/** 캐릭터가 Dead 상태인지 여부 */
	UPROPERTY(BlueprintReadOnly)
	uint8 bIsDead : 1;

	/** 모델의 경우 언리얼의 XYZ 좌표계와 다르기 때문에 Pitch에 적용할 경우 Yaw에 적용된다. */
	UPROPERTY(BlueprintReadOnly)
	float ModifyPitch;

	/** 모델의 경우 언리얼의 XYZ 좌표계와 다르기 때문에 Yaw에 적용할 경우 Pitch에 적용된다. */
	UPROPERTY(BlueprintReadOnly)
	float ModifyYaw;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float StrafeYaw = 15.0f;
	
	/** Lean을 적용할 최소 속도, 크롤로 움직일 시점부터 Lean을 적용해야 한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float LeanThresholdSpeed;

	/** Lean을 적용할 최대 Pitch 각도 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxPitchAngle;

	/** Lean을 적용할 최대 Yaw 각도 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxYawAngle;

	/** Forward 방향으로 이동할 때의 허용 범위. -90, 90을 정면으로 인식하지 않게 하기 위한 범위이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ForwardDirectionTolerance;
	
	/** Lean 적용 여부 */
	UPROPERTY(BlueprintReadOnly)
	uint8 bShouldLean : 1;

#pragma endregion

#pragma region Getter Setter

	UFUNCTION(BlueprintCallable)
	FORCEINLINE AUnderwaterCharacter* GetUnderwaterCharacter() const
	{
		return UnderwaterCharacter.Get();
	}

	UFUNCTION(BlueprintCallable)
	FORCEINLINE UCharacterMovementComponent* GetCharacterMovement() const
	{
		return CharacterMovement.Get();
	}
	
#pragma endregion
};
