// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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

	void UpdateVariables();
	
#pragma endregion
	
#pragma region Variable

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<class AUnderwaterCharacter> UnderwaterCharacter;

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<class UCharacterMovementComponent> CharacterMovement;

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
	bool bShouldMove;

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
