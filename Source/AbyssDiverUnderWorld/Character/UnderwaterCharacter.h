// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "UnitBase.h"
#include "UnderwaterCharacter.generated.h"

// 지상, 수중을 하나의 클래스로 구현하기로 결정
// 하나의 클래스로 구현하는 이유는 수중, 지상을 이동할 수도 있는 기능을 지원해야 할 수도 있기 때문이다.
// 새로 Respawn하면 되기는 하지만 그럴 경우 데이터를 누락할 수 있다.
// 그러한 상황을 방지하고 중복 구현을 방지하기 위해 하나의 클래스로 구현한다.
// 수중과 지상은 상태 머신으로 처리해야 할 정도로 복잡한 전이가 아니기 때문에 enum을 통해서 구현한다.

/* 캐릭터의 지상, 수중을 결정, Move 로직, Animation이 변경되고 사용 가능 기능이 제한된다. */
UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	Underwater,
	Ground,
};

class UInputAction;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AUnderwaterCharacter : public AUnitBase
{
	GENERATED_BODY()

public:
	AUnderwaterCharacter();

protected:
	virtual void BeginPlay() override;

	/** IA를 Enhanced Input Component에 연결 */
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

#pragma region Method

public:
	/** 현재 캐릭터의 상태를 전환. 수중, 지상 */
	UFUNCTION(BlueprintCallable)
	void SetCharacterState(ECharacterState State);

	/** 빠른 구현을 위해 Captrue를 현재 Multicast로 구현한다.
	 * 이후 변경 모델
	 * 1. Replicate 모델 사용 : Replicate 모델은 변수를 기반으로 작동하기 때문에 중도 참여자도 현재 상태를 판단할 수 있다.
	 * 혹은 2개가 결합된 방식을 사용할 수 있다.
	 * 2. State Pattern 사용 : 상태가 많아질 경우 해당 State Pattern을 사용해서 상태를 관리한다.
	 */
	
	/** 캐릭터 Capture 상태 실행 */
	UFUNCTION(BlueprintCallable)
	void StartCaptureState();

	/** 캐릭터 Capture 상태 종료 */
	UFUNCTION(BlueprintCallable)
	void StopCaptureState();

	UFUNCTION(BlueprintCallable)
	void EmitBloodNoise();
	
protected:
	/** Capture State Multicast
	 * Owner : 암전 효과, 입력 처리
	 * All : Mesh 비활성화
	 */
	UFUNCTION(NetMulticast, Reliable)
	void M_StartCaptureState();
	void M_StartCaptureState_Implementation();

	/** Capture State Multicast
	 * Owner : 암전 효과, 입력 처리
	 * All : Mesh 활성화
	 */
	UFUNCTION(NetMulticast, Reliable)
	void M_StopCaptureState();
	void M_StopCaptureState_Implementation();

	UFUNCTION()
	void AdjustSpeed();

	/** 이동 함수. 지상, 수중 상태에 따라 이동한다. */
	void Move(const FInputActionValue& InputActionValue);

	/** 수중 이동 함수. Forward : Camera 방향으로 이동, Right : Forward 기준을 바탕으로 왼쪽, 오른쪽 수평 이동, Up : 위쪽 수직 이동 */
	void MoveUnderwater(FVector MoveInput);

	/** 지상 이동 함수 */
	void MoveGround(FVector MoveInput);

	/** 스프린트 시작 함수. Stamina가 감소하기 시작한다. */
	void StartSprint(const FInputActionValue& InputActionValue);

	/** 스프린트 종료 함수. Stamina 회복이 시작된다. */
	void StopSprint(const FInputActionValue& InputActionValue);

	UFUNCTION()
	void OnSprintStateChanged(bool bNewSprinting);

	/** 회전 함수. 현재 버전은 Pitch가 제한되어 있다. */
	void Look(const FInputActionValue& InputActionValue);

	/** 발사 함수. 미구현*/
	void Fire(const FInputActionValue& InputActionValue);

	/** 조준 함수. 미구현*/
	void Aim(const FInputActionValue& InputActionValue);

	/** 상호작용 함수. Interaction의 Focus Interactable과 상호작용을 실행한다. 현재는 Start를 기점으로 작동 */
	void Interaction(const FInputActionValue& InputActionValue);

	/** 라이트 함수. 미구현*/
	void Light(const FInputActionValue& InputActionValue);

	/** 레이더 함수. 미구현*/
	void Radar(const FInputActionValue& InputActionValue);

	/** 3인칭 디버그 카메라 활성화 설정 */
	void SetDebugCameraMode(bool bDebugCameraEnable);

	/** 디버그 카메라 모드 토글. 1인칭, 3인칭을 전환한다. */
	UFUNCTION(CallInEditor)
	void ToggleDebugCameraMode();

#pragma endregion

#pragma region Variable

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Debug, meta = (AllowPrivateAccess = "true"))
	uint8 bUseDebugCamera : 1;

	/** 캐릭터의 현재 상태. 지상 혹은 수중 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	ECharacterState CharacterState;

	/** Capture 상태 여부. 추후 상태가 많아지면 상태 패턴 이용을 고려 */
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	uint8 bIsCaptured : 1;

	UPROPERTY(EditAnywhere, Category = Character, meta = (AllowPrivateAccess = "true"))
	float CaptureFadeTime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	float BloodEmitNoiseRadius;

	/** 초과 적재 기준 무게. 초과할 경우 속도를 감소시킨다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	float OverloadWeight;

	/** 초과 적재 시의 속도 감소 비율. [0, 1]의 범위로 속도를 감소시킨다. 0.4일 경우 40% 감소해서 60%의 속도이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float OverloadSpeedFactor;

	UPROPERTY(Transient, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	float EffectiveSpeed;
	
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	float SprintSpeed;

	/** 이동 입력, 3차원 입력을 받는다. 캐릭터의 XYZ 축대로 맵핑을 한다. Forward : X, Right : Y, Up : Z */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SprintAction;

	/** 마우스 회전 입력, 2차원 입력을 받는다. 기본적으로 Y축을 반전해서 들어온다. Right : X, Up : -Y */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	/** 발사 입력 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> FireAction;

	/** 조준 입력 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> AimAction;

	/** 상호작용 입력 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> InteractionAction;

	/** 라이트 입력 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LightAction;

	/** 레이더 입력 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> RadarAction;

	/** 게임에 사용될 1인칭 Camera Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> FirstPersonCameraComponent;

	/** 디버깅 목적으로 사용할 3인칭 Camera Component의 Spring Arm */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> CameraBoom;

	/** 디버깅 목적으로 사용될 3인칭 Camera Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> ThirdPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, Category = Mesh , meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USkeletalMeshComponent> Mesh1P;

	/** 캐릭터의 산소 상태를 관리하는 Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UOxygenComponent> OxygenComponent;

	/** 캐릭터의 스태미너 상태를 관리하는 Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaminaComponent> StaminaComponent;

	/** 캐릭터 출혈 시를 시뮬레이션을 하는 Noise Emitter Component */
	UPROPERTY()
	TObjectPtr<class UPawnNoiseEmitterComponent> NoiseEmitterComponent;
	
	/** 상호작용 실행하게 하는 Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UADInteractionComponent> InteractionComponent;

	/** Shop의 Interaction을 실행할 Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UShopInteractionComponent> ShopInteractionComponent;

	TWeakObjectPtr<class UADInventoryComponent> InventoryComponent;
	
#pragma endregion

#pragma region Getter Setter

public:
	/** Interaction Component를 반환 */
	FORCEINLINE UADInteractionComponent* GetInteractionComponent() const { return InteractionComponent; }

	/** Shop Interaction Component를 반환 */
	FORCEINLINE UShopInteractionComponent* GetShopInteractionComponent() const { return ShopInteractionComponent; }

	/** 캐릭터의 상태를 반환 */
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }

	FORCEINLINE bool IsSprinting() const;

	UFUNCTION(BlueprintCallable)
	bool IsOverloaded() const;
	
#pragma endregion
};
