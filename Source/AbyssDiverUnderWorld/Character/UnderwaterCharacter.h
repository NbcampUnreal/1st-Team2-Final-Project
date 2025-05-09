// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "UnitBase.h"
#include "UnderwaterCharacter.generated.h"

class UInputAction;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AUnderwaterCharacter : public AUnitBase
{
	GENERATED_BODY()

public:
	AUnderwaterCharacter();

protected:
	virtual void BeginPlay() override;

#pragma region Method

	/** IA를 Enhanced Input Component에 연결 */
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	/** 이동 함수. Forward : Camera 방향으로 이동, Right : Forward 기준을 바탕으로 왼쪽, 오른쪽 수평 이동, Up : 위쪽 수직 이동 */
	void Move(const FInputActionValue& InputActionValue);

	/** 회전 함수. 현재 버전은 Pitch가 제한되어 있다. */
	void Look(const FInputActionValue& InputActionValue);

	/** 발사 함수. 미구현*/
	void Fire(const FInputActionValue& InputActionValue);

	/** 조준 함수. 미구현*/
	void Aim(const FInputActionValue& InputActionValue);

	/** 상호작용 함수. 미구현*/
	void Interaction(const FInputActionValue& InputActionValue);

	/** 라이트 함수. 미구현*/
	void Light(const FInputActionValue& InputActionValue);

	/** 레이더 함수. 미구현*/
	void Radar(const FInputActionValue& InputActionValue);
	
#pragma endregion
	
#pragma region Variable
	
private:
	/** 이동 입력, 3차원 입력을 받는다. 캐릭터의 XYZ 축대로 맵핑을 한다. Forward : X, Right : Y, Up : Z */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	/** 마우스 회전 입력, 2차원 입력을 받는다. 기본적으로 Y축을 반전해서 들어온다. Right : X, Up : -Y */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	/** 발사 입력 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> FireAction;

	/** 조준 입력 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> AimAction;

	/** 상호작용 입력 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> InteractionAction;

	/** 라이트 입력 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LightAction;

	/** 레이더 입력 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> RadarAction;

	/** 게임에 사용될 1인칭 Camera Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> FirstPersonCameraComponent;
	
#pragma endregion
	
};
