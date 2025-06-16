// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "UnitBase.h"
#include "Interface/IADInteractable.h"
#include "AbyssDiverUnderWorld.h"
#include "StatComponent.h"
#include "UnderwaterCharacter.generated.h"

#if UE_BUILD_SHIPPING
	#define LOG_ABYSS_DIVER_COMPILE_VERBOSITY Error
#else
	#define LOG_ABYSS_DIVER_COMPILE_VERBOSITY All
#endif

#define LOG_NETWORK(Category, Verbosity, Format, ...) \
	UE_LOG(Category, Verbosity, TEXT("[%s] %s %s"), LOG_NETMODEINFO, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))

DECLARE_LOG_CATEGORY_EXTERN(LogAbyssDiverCharacter, Log, LOG_ABYSS_DIVER_COMPILE_VERBOSITY);

// @TODO : Character Status Replicate 문제
// 1. 최대 Stamina는 최대 Oxygen에 비례하는데 값이 따로따로 Replicate될 수 있다. 이렇게 되면 항상 오차가 존재하게 된다.
// 2. 매 프레임 변수가 Replicate 되는 상황이 발생한다. 4인 유저 체제일 경우 어느정도 대역폭을 소모하게 될 지 모른다.
// 실제 게임 플레이 태스트와 프로파일링을 통해서 문제를 해결해야 한다.
// 3. Stamina, Oxygen 컴포넌트가 분리되어서 더 복잡해지고 있는 상황일 수 있다. 추후 구현이 필요 이상으로 복잡해지면 합치는 것을 고려한다.

// @TODO : 수중 캐릭터와 지상 캐릭터 분리
// 만약에 레벨 전환이 있다고 가정하면 새로 캐릭터를 분리하는 것이 덜 복잡하게 된다.
// 이 부분을 문의하고 확정된 스펙에 따라 결정한다.

enum class ELocomotionMode : uint8;

/* 캐릭터의 지상, 수중을 결정, Move 로직, Animation이 변경되고 사용 가능 기능이 제한된다. */
UENUM(BlueprintType)
enum class EEnvironmentState : uint8
{
	Underwater,
	Ground,
};

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	Normal UMETA(DisplayName = "Normal"),
	Groggy UMETA(DisplayName = "Groggy"),
	Death UMETA(DisplayName = "Death"),
};

// FSM으로 구현했습니다. 추후에 State Pattern으로 변경 고려하겠습니다.
// - Server Logic : HandleServer
// - Client Logic : HandleClient
// - Multicast Logic : M_NotifyStateChange
// 공통 로직이 Multicast에서 설정이 되므로 Multicast에서 이벤트를 발생하겠습니다.

// 1. Normal -> Groggy : Health == 0
// 2. Normal -> Death : Oxygen == 0
// 3. Groggy -> Death : Oxygen == 0
// 4. Groggy -> Death : Groggy Time Out
// 5. Groggy -> Normal : Revive

USTRUCT(BlueprintType)
struct FAnimSyncState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 bEnableRightHandIK : 1 = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 bEnableLeftHandIK : 1 = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 bEnableFootIK : 1 = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 bIsStrafing : 1 = false;
};

UENUM(BlueprintType)
enum class EPlayAnimationTarget : uint8
{
	FirstPersonMesh UMETA(DisplayName = "First Person"),
	ThirdPersonMesh UMETA(DisplayName = "Third Person"),
	BothPersonMesh UMETA(DisplayName = "Both Person"),
};

class UInputAction;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AUnderwaterCharacter : public AUnitBase, public IIADInteractable
{
	GENERATED_BODY()

public:
	AUnderwaterCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void PostNetInit() override;
	virtual void OnRep_PlayerState() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;

	/** IA를 Enhanced Input Component에 연결 */
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

#pragma region Method

public:
	// Interactable Interface Begin

	/** Interact 함수가 호출되면 실행되는 함수 */
	virtual void Interact_Implementation(AActor* InstigatorActor) override;
	
	/** Interact Hold됬을 때 호출될 함수 */
	virtual void InteractHold_Implementation(AActor* InstigatorActor) override;

	/** Interact Hold가 시작됬을 때 호출될 함수 */
	virtual void OnHoldStart_Implementation(APawn* InstigatorPawn) override;

	/** Interact Hold가 중간에 멈췄을 때 호출될 함수 */
	virtual void OnHoldStop_Implementation(APawn* InstigatorPawn) override;
	
	/** Interact Highlight 출력 여부 */
	virtual bool CanHighlight_Implementation() const override;

	/** Hold 지속 시간 반환 */
	virtual float GetHoldDuration_Implementation() const override;
	
	/** Interactable 컴포넌트를 반환 */
	virtual UADInteractableComponent* GetInteractableComponent() const override;

	/** Interactable Hold 모드 설정 */
	virtual bool IsHoldMode() const override;

	/** 상호작용 타입 반환 */
	virtual FString GetInteractionDescription() const override;
	
	// Interactable Interface End

	// Launch Character
	// - Boss: 공격 시에 넉백을 적용하기 Launch를 실행한다.
	// - CurrentZone : 급류 효과를 위해 Launch를 실행한다.
	// - SpikeHazard : 공격 효과를 위해 Launch를 실행한다.
	
	/** Launch Character를 오버라이드 해서 캐릭터의 넉백 상태를 확인한다. */
	virtual void LaunchCharacter(FVector LaunchVelocity, bool bXYOverride, bool bZOverride) override;
	
	/** 그로기 상태 캐릭터를 부활시킨다. */
	UFUNCTION(BlueprintCallable)
	void RequestRevive();

	/** 현재 캐릭터의 상태를 전환. 수중, 지상 */
	UFUNCTION(BlueprintCallable)
	void SetEnvironmentState(EEnvironmentState State);

	// Monster 인식 시에 Target
	// Monster가 놓칠 시에 UnTarget
	// Monster가 사망했을 때 UnTarget
	// Monster가 Target을 변경했을 때 UnTarget

	// 추후, Combat State 진입 시에 로컬 효과가 존재한다면 Replicate를 통해서 전파한다.
	
	/** Monster에 의해 Target 되었을 때 호출된다. Authority Node에서만 유효하다. */
	UFUNCTION(BlueprintCallable)
	void OnTargeted();

	/** Monster에 의해 UnTarget 되었을 때 호출된다. Authority Node에서만 유효하다. */
	UFUNCTION(BlueprintCallable)
	void OnUntargeted();

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

	/** 출혈을 모델링하는 소리를 발생한다. */
	UFUNCTION(BlueprintCallable)
	void EmitBloodNoise();

	/** 캐릭터의 몽타주 재생 요청 */
	void RequestPlayMontage(UAnimMontage* Mesh1PMontage, UAnimMontage* Mesh3PMontage, float InPlayRate = 1.0f, FName StartSectionName = NAME_None);

	/** 서버에 몽타주 재생 요청 */
	UFUNCTION(Reliable, Server)
	void S_PlayMontage(UAnimMontage* Mesh1PMontage, UAnimMontage* Mesh3PMontage, float InPlayRate = 1.0f, FName StartSectionName = NAME_None);
	void S_PlayMontage_Implementation(UAnimMontage* Mesh1PMontage, UAnimMontage* Mesh3PMontage, float InPlayRate = 1.0f, FName StartSectionName = NAME_None);

	/** 몽타주 재생 전파 */
	UFUNCTION(NetMulticast, Reliable)
	void M_BroadcastPlayMontage(UAnimMontage* Mesh1PMontage, UAnimMontage* Mesh3PMontage, float InPlayRate = 1.0f, FName StartSectionName = NAME_None);
	void M_BroadcastPlayMontage_Implementation(UAnimMontage* Mesh1PMontage, UAnimMontage* Mesh3PMontage, float InPlayRate = 1.0f, FName StartSectionName = NAME_None);

	/** 현재 재생 중인 몽타주 정지 요청 */
	void RequestStopAllMontage(EPlayAnimationTarget Target, float BlendOut = 0.0f);

	/** 몽타주 정지 Server RPC */
	UFUNCTION(Reliable, Server)
	void S_StopAllMontage(EPlayAnimationTarget Target, float BlendOut = 0.0f);
	void S_StopAllMontage_Implementation(EPlayAnimationTarget Target, float BlendOut = 0.0f);

	/** 몽타주 정지 전파 */
	UFUNCTION(NetMulticast, Reliable)
	void M_StopAllMontage(EPlayAnimationTarget Target, float BlendOut = 0.0f);
	void M_StopAllMontage_Implementation(EPlayAnimationTarget Target, float BlendOut = 0.0f);
	
	/** 1인칭 메시, 3인칭 메시 모두에 애니메이션 몽타주를 재생한다. */
	UFUNCTION(NetMulticast, Reliable)
	void M_PlayMontageOnBothMesh(UAnimMontage* Montage, float InPlayRate = 1.0f, FName StartSectionName = NAME_None, FAnimSyncState NewAnimSyncState = FAnimSyncState());
	void M_PlayMontageOnBothMesh_Implementation(UAnimMontage* Montage, float InPlayRate = 1.0f, FName StartSectionName = NAME_None, FAnimSyncState NewAnimSyncState = FAnimSyncState());

	/** 1인칭 메시, 3인칭 메시 모두 애니메이션 몽타주를 정지한다. */
	UFUNCTION(NetMulticast, Reliable)
	void M_StopAllMontagesOnBothMesh(float BlendOut);
	void M_StopAllMontagesOnBothMesh_Implementation(float BlendOut);

	/** Anim State 변경 요청 */
	void RequestChangeAnimSyncState(FAnimSyncState NewAnimSyncState);

	/** AnimNotify_LaserCutter 가 호출하는 정리 함수 */
	UFUNCTION(BlueprintCallable, Category = "Mining")
	void CleanupToolAndEffects();

	/** 암반이 요청하면 Mining Tool을 스폰해 착용하는 함수 */
	void SpawnAndAttachTool(TSubclassOf<AActor> ToolClass);

	UFUNCTION()
	void OnRep_CurrentTool();
	
protected:

	/** Stat Component의 기본 속도가 변경됬을 때 호출된다. */
	UFUNCTION()
	void OnMoveSpeedChanged(float NewMoveSpeed);

    /** Jump 가능 상태를 반환 */
	virtual bool CanJumpInternal_Implementation() const override;
	
	/** 메시 컴포넌트를 동적으로 생성하고 Parent 에 소켓으로 연결한다.*/
	UStaticMeshComponent* CreateAndAttachMesh(const FString& ComponentName, UStaticMesh* MeshAsset, USceneComponent* Parent, FName SocketName, bool bIsThirdPerson);

	/** 오리발 메시를 생성한다. 1인칭, 3인칭 모두에 적용된다. */
	void SpawnFlipperMesh();

	/** 오리발 메시의 Visibility를 설정한다. */
	void SetFlipperMeshVisibility(bool bVisible);
	
	/** Anim State 변경 Server RPC */
	UFUNCTION(Server, Reliable)
	void S_ChangeAnimSyncState(FAnimSyncState NewAnimSyncState);
	void S_ChangeAnimSyncState_Implementation(FAnimSyncState NewAnimSyncState);

	/** Anim State 변경 Multicast RPC */
	UFUNCTION(NetMulticast, Reliable)
	void M_UpdateAnimSyncState(FAnimSyncState NewAnimSyncState);
	void M_UpdateAnimSyncState_Implementation(FAnimSyncState NewAnimSyncState);
	
	/** 캐릭터 상태를 설정한다. Server에서만 실행 가능하다. */
	void SetCharacterState(ECharacterState NewCharacterState);

	/** Server에서 설정한 Multicast를 전파한다. 전파 중에 다시 상태를 전이하면 안 된다. */
	UFUNCTION(NetMulticast, Reliable)
	void M_NotifyStateChange(ECharacterState NewCharacterState);
	void M_NotifyStateChange_Implementation(ECharacterState NewCharacterState);

	/** State 바뀔 때 새로운 State Enter 적용 */
	void HandleEnterState(ECharacterState HandleCharacterState);

	/** State 바뀔 때 이전 State Exit 적용 */
	void HandleExitState(ECharacterState HandleCharacterState);

	/** Groggy 상태 진입. Groggy Time 동안 Groggy 시간에 진입하게 되고 Groggy 시간이 지나면 사망한다. */
	void HandleEnterGroggy();

	/** Groggy 상태 종료. */
	void HandleExitGroggy();

	/** Normal 상태 진입. 이동이 가능하고 Stamina, Oxygen 기능이 활성화 */
	void HandleEnterNormal();

	/** Stamina, Oxygen 기능이 비활성화 */
	void HandleExitNormal();

	/** Death 상태 진입. 사망 처리 */
	void HandleEnterDeath();

	/** 서버에서 부활 요청을 받았을 때 호출되는 함수 */
	UFUNCTION(Server, Reliable)
	void S_Revive();
	void S_Revive_Implementation();

	/** Groggy 상태에서 사망까지 걸리는 시간을 계산한다. 사망을 할 수록 기간이 길어진다. */
	virtual float CalculateGroggyTime(float CurrentGroggyDuration, uint8 CalculateGroggyCount) const;
	
	/** 캐릭터 사망 시에 Blueprint에서 호출될 함수 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnDeath"))
	void K2_OnDeath();

	/** 캐릭터의 환경이 변경됬을 시에 Blueprint에서 호출될 함수 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnEnvironmentStateChanged"))
	void K2_OnEnvironmentStateChanged(EEnvironmentState OldEnvironmentState, EEnvironmentState NewEnvironmentState);

	/** Player State 정보를 초기화 */
	void InitFromPlayerState(class AADPlayerState* ADPlayerState);
	
	/** Upgrade Component의 정보를 바탕으로 초기화 */
	void ApplyUpgradeFactor(class UUpgradeComponent* UpgradeComponent);
	
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

	// Stat Component의 회복 기능을 통해서 구현한다.
	
	// - Combat End
	// - (UnCombat) Damage를 받고 일정 시간 이후에 체력 회복 시작
	
	/** Health Regen을 시작한다. */
	void StartHealthRegen();

	// - Combat Start
	// - (UnCombat) Damage를 받고 일정 시간 이후에 체력 회복 중지
	/** Health Regen을 종료 */
	void StopHealthRegen();
	
	/** 전투 진입 시에 호출되는 함수 */
	void StartCombat();

	/** 전투 종료 시에 호출되는 함수 */
	void EndCombat();
	
	/** 현재 상태 속도 갱신.(무게, Sprint) */
	UFUNCTION()
	void AdjustSpeed();

	/** 레이더 Actor를 생성한다. */
	void SpawnRadar();

	/** Radar Toggle을 요청한다. */
	UFUNCTION(BlueprintCallable)
	void RequestToggleRadar();

	/** 현재 상태에 맞춰서 Blur 효과를 업데이트한다. */
	void UpdateBlurEffect();
	
	/** Blur 효과 적용 여부를 설정한다. */
	void SetBlurEffect(const bool bEnable);

	/** Radar 보이는 것을 설정 */
	void SetRadarVisibility(bool bRadarVisible);

	/** Toggle Radar Server RPC */
	UFUNCTION(Server, Reliable)
	void S_ToggleRadar();
	void S_ToggleRadar_Implementation();

	/** bIsRadarOn Replicate 함수. 변화된 값에 따라서 레이더의 Visibility를 변경한다. */
	UFUNCTION()
	void OnRep_bIsRadarOn();

	/** 산소 상태가 변경될 때 호출되는 함수 */
	UFUNCTION()
	void OnOxygenLevelChanged(float CurrentOxygenLevel, float MaxOxygenLevel);

	/** 산소가 소진되었을 때 호출되는 함수 */
	UFUNCTION()
	void OnOxygenDepleted();

	/** 체력 상태가 변경될 때 호출되는 함수 */
	UFUNCTION()
	void OnHealthChanged(int32 CurrentHealth, int32 MaxHealth);

	/** 물리 볼륨이 변경되었을 때 호출되는 함수 */
	UFUNCTION()
	void OnPhysicsVolumeChanged(class APhysicsVolume* NewVolume);
	
	/** 이동 함수. 지상, 수중 상태에 따라 이동한다. */
	void Move(const FInputActionValue& InputActionValue);

	/** 수중 이동 함수. Forward : Camera 방향으로 이동, Right : Forward 기준을 바탕으로 왼쪽, 오른쪽 수평 이동, Up : 위쪽 수직 이동 */
	void MoveUnderwater(FVector MoveInput);

	/** 지상 이동 함수 */
	void MoveGround(FVector MoveInput);

	/** 점프 입력 시작 함수. 지상에서만 작동한다. */
	void JumpInputStart(const FInputActionValue& InputActionValue);

	/** 점프 입력 중단 함수. 지상에서만 작동한다. */
	void JumpInputStop(const FInputActionValue& InputActionValue);
	
	/** Jump 될 때 호출되는 함수. Jump State를 추적한다. */
	virtual void OnJumped_Implementation() override;

	/** Land 될 때 호출되는 함수. Jump State를 추적한다. */
	virtual void Landed(const FHitResult& Hit) override;

	/** 스프린트 시작 함수. Stamina가 감소하기 시작한다. */
	void StartSprint(const FInputActionValue& InputActionValue);

	/** 스프린트 종료 함수. Stamina 회복이 시작된다. */
	void StopSprint(const FInputActionValue& InputActionValue);

	/** Sprint 상태 변화 시에 호출되는 함수 */
	UFUNCTION()
	void OnSprintStateChanged(bool bNewSprinting);

	/** 회전 함수. 현재 버전은 Pitch가 제한되어 있다. */
	void Look(const FInputActionValue& InputActionValue);

	/** 발사 함수. 미구현*/
	void Fire(const FInputActionValue& InputActionValue);

	/** 발사 중지 함수. 미구현*/
	void StopFire(const FInputActionValue& InputActionValue);

	/** 조준 함수. 미구현*/
	void Aim(const FInputActionValue& InputActionValue);

	/** 상호작용 함수. Interaction의 Focus Interactable과 상호작용을 실행한다. 현재는 Start를 기점으로 작동 */
	void Interaction(const FInputActionValue& InputActionValue);

	void CompleteInteraction(const FInputActionValue& InputActionValue);

	/** 라이트 토글 함수*/
	void Light(const FInputActionValue& InputActionValue);

	/** 레이더 토글 함수 */
	void Radar(const FInputActionValue& InputActionValue);

	/** 재장전 함수 */
	void Reload(const FInputActionValue& InputActionValue);

	void CompleteReload(const FInputActionValue& InputActionValue);

	/** 1번 슬롯 장착 함수 */
	void EquipSlot1(const FInputActionValue& InputActionValue);

	/** 2번 슬롯 장착 함수 */
	void EquipSlot2(const FInputActionValue& InputActionValue);

	/** 3번 슬롯 장착 함수 */
	void EquipSlot3(const FInputActionValue& InputActionValue);

	/** 1번 감정 표현 실행 */
	void PerformEmote1(const FInputActionValue& InputActionValue);

	/** 2번 감정 표현 실행 */
	void PerformEmote2(const FInputActionValue& InputActionValue);

	/** 3번 감정 표현 실행 */
	void PerformEmote3(const FInputActionValue& InputActionValue);
	
	/** 3인칭 디버그 카메라 활성화 설정 */
	void SetDebugCameraMode(bool bDebugCameraEnable);

	/** 디버그 카메라 모드 토글. 1인칭, 3인칭을 전환한다. */
	UFUNCTION(CallInEditor)
	void ToggleDebugCameraMode();

	/** 1인칭 메시 몽타주 시작 시 호출되는 함수 */
	UFUNCTION()
	virtual void OnMesh1PMontageStarted(UAnimMontage* Montage);

	/** 1인칭 메시 몽타주 종료 시 호출되는 함수 */
	UFUNCTION()
	virtual void OnMesh1PMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	/** 3인칭 메시 몽타주 시작 시 호출되는 함수 */
	UFUNCTION()
	virtual void OnMesh3PMontageStarted(UAnimMontage* Montage);

	/** 3인칭 메시 몽타주 종료 시 호출되는 함수 */
	UFUNCTION()
	virtual void OnMesh3PMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	/** 감정 표현을 재생. Local에서 실행된다. */
	void PlayEmote(uint8 EmoteIndex);

	/** 감정 표현을 중지 */
	void StopPlayingEmote();

	/** 감정 표현 몽타주가 끝났을 때 호출되는 함수 */
	UFUNCTION()
	void OnEmoteEnd(UAnimMontage* AnimMontage, bool bArg);
	
private:

	/** Montage 콜백을 등록 */
	void SetupMontageCallbacks();
	
#pragma endregion

#pragma region Variable

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
	/** 캐릭터가 사망했을 때 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable)
	FOnDeath OnDeathDelegate;

	// Knockback이 되는 상황은 LaunchCharacter 때이므로 LaunchCharacter를 오버라이드해서 처리한다.
	// 복귀가 될 때는 MoveMode 변화를 통해서 처리한다.
	// Launch Character는 Client에서도 호출되므로 Local 효과를 재생할 수 있다.
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKnockbacked, FVector, KnockbackVelocity);
	/** 캐릭터가 넉백되었을 때 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable)
	FOnKnockbacked OnKnockbackDelegate;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnKnockbackEnd);
	/** 캐릭터의 넉백이 끝났을 때 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable)
	FOnKnockbackEnd OnKnockbackEndDelegate;
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGroggy);
	/** 캐릭터가 그로기 상태에 진입했을 때 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable)
	FOnGroggy OnGroggyDelegate;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterStateChanged, ECharacterState, OldCharacterState, ECharacterState, NewCharacterState);
	/** 캐릭터 상태가 변경되었을 때 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable)
	FOnCharacterStateChanged OnCharacterStateChangedDelegate;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnvironmentStateChanged, EEnvironmentState, OldEnvironmentState, EEnvironmentState, NewEnvironmentState);
	/** 캐릭터의 환경 상태가 변경되었을 때 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable)
	FOnEnvironmentStateChanged OnEnvironmentStateChangedDelegate;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageTaken, float, DamageAmount, float, CurrentHealth);
	/** 캐릭터가 피해를 입었을 때 호출되는 델리게이트, DamageAmount = Health Damage Taken + Shield Damage Taken */
	UPROPERTY(BlueprintAssignable)
	FOnDamageTaken OnDamageTakenDelegate;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnComatStart);
	/** 캐릭터가 전투 상태에 진입했을 때 호출되는 델리게이트, Server에서만 호출 */
	UPROPERTY(BlueprintAssignable)
	FOnComatStart OnCombatStartDelegate;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCombatEnd);
	/** 캐릭터가 전투 상태에서 벗어났을 때 호출되는 델리게이트, Server에서만 호출 */
	UPROPERTY(BlueprintAssignable)
	FOnCombatEnd OnCombatEndDelegate;
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMontageEnd, UAnimMontage*, Montage, bool, bInterrupted);
	/** 1인칭 메시 몽타주 종료 시 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = Animation)
	FOnMontageEnd OnMesh1PMontageEndDelegate;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMontageStarted, UAnimMontage*, Montage);
	/** 1인칭 메시 몽타주 시작 시 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = Animation)
	FOnMontageStarted OnMesh1PMontageStartedDelegate;

	UPROPERTY(BlueprintAssignable, Category = Animation)
	/** 3인칭 메시 몽타주 종료 시 호출되는 델리게이트 */
	FOnMontageEnd OnMesh3PMontageEndDelegate;

	UPROPERTY(BlueprintAssignable, Category = Animation)
	/** 3인칭 메시 몽타주 시작 시 호출되는 델리게이트 */
	FOnMontageStarted OnMesh3PMontageStartedDelegate;

	UPROPERTY(VisibleAnywhere, Category = "Mining")
	/** 현재 1p에 장착된 Tool 인스턴스 */
	TObjectPtr<AActor> SpawnedTool;

	TObjectPtr<USkeletalMeshComponent> CachedSkeletalMesh;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentTool)
	TObjectPtr<AActor> CurrentTool = nullptr;

	UPROPERTY()
	TObjectPtr<AActor> PrevTool = nullptr;
	
	UPROPERTY(VisibleAnywhere, Category = "Mining")
	/** 현재 1p에 장착된 Tool 인스턴스 */
	TObjectPtr<AActor> SpawnedTool1P;
	
	UPROPERTY(VisibleAnywhere, Category = "Mining")
	/** 현재 3p에 장착된 Tool 인스턴스 */
	TObjectPtr<AActor> SpawnedTool3P;

private:

	// Character State는 현재 State 종료 시에 따로 처리할 것이 없기 때문에 현재 상태 값만 Replicate하도록 한다.
	
	/* 현재 캐릭터 상태. Normal, Groggy, Death... */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	ECharacterState CharacterState;

	/** 캐릭터의 현재 로코모션 상태. Jump인지 그냥 Fall인지를 구분한다. */
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	ELocomotionMode LocomotionMode;

	/** 가장 나중에 착지한 시간, 점프를 Block할 수 있는 기점이 된다 */
	float LastLandedTime;

	/** 캐릭터가 Land 시에 점프를 Block할 시간. 캐릭터가 Land 모션 중에서 점프하는 것을 방지한다. */
	UPROPERTY(EditDefaultsOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	float LandedJumpBlockTime;

	/** Global Gravity Z에 상관 없이 캐릭터가 설정할 Gravity Z 값. 음수값을 지정해야 한다. */
	UPROPERTY(EditDefaultsOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	float ExpectedGravityZ;

	/** 플레이어 무적 설정. 무적이 되면 피해를 입지 않는다. */
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	uint8 bIsInvincible : 1;
	
	/** Normal 상태에서 장비 착용 가능 여부 */
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	uint8 bCanUseEquipment : 1;

	/** 감정 표현 중 여부, Client 에서만 저장하고 따로 전파하지 않는다. */
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	uint8 bPlayingEmote : 1;
	
	/** 캐릭터 랜턴의 거리 */
	UPROPERTY(EditDefaultsOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	float LanternLength;
	
	/** 오리발이 생성될 왼발 소켓 이름 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|Flipper")
	FName LeftFlipperSocketName;
	
	/** 왼발에 착용될 오리발 메시 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|Flipper")
	TObjectPtr<UStaticMesh> LeftFlipperMesh;

	/** 왼발에 착용될 오리발 메시 1인칭용 컴포넌트 */
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> LeftFlipperMesh1PComponent;

	/** 왼발에 착용될 오리발 메시 3인칭용 컴포넌트 */
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> LeftFlipperMesh3PComponent;

	/** 오리발이 생성될 오른발 소켓 이름 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|Flipper")
	FName RightFlipperSocketName;
	
	/** 오른발에 착용될 오리발 메시 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|Flipper")
	TObjectPtr<UStaticMesh> RightFlipperMesh;

	/** 오른발에 착용될 오리발 메시 1인칭용 컴포넌트 */
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> RightFlipperMesh1PComponent;

	/** 오른발에 착용될 오리발 메시 3인칭용 컴포넌트 */
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> RightFlipperMesh3PComponent;

	/** 현재 회전 감도. 현재 상태에 따라 변화 한다. */
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	float LookSensitivity;

	/** Normal 상태에서의 회전 감도. Normal 상태에 진입할 때마다 LookSensitivity를 이 값으로 설정한다. */
	UPROPERTY(EditDefaultsOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	float NormalLookSensitivity;
	
	/** 그로기 상태에서 사망까지 걸리는 시간. 그로기 상태에 진입할 떄마다 줄어든다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Groggy", meta = (AllowPrivateAccess = "true"))
	float GroggyDuration;

	/** 그로기 상태에서 공격 받았을 경우 줄어드는 Groggy 시간. 초 단위이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Groggy", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float GroggyHitPenalty;

	/** 그로기 상태에 진입될 떄마다 감소하는 Groggy Duration 비율. [0, 1]의 범위로 설정한다. 0.3일 경우 0.7배로 감소한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Groggy", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float GroggyReductionRate;

	/** 그로기 상태를 계산할 때 최소 그로기 상태 시간. GroggyDuration * GroggyReductionRate보다 작을 경우 이 값으로 설정한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Groggy", meta = (AllowPrivateAccess = "true"))
	float MinGroggyDuration;

	/** 그로기 상태에 진입한 횟수. uint8이라서 오버플로우에 주의 */
	UPROPERTY(BlueprintReadOnly, Category = "Character|Groggy", meta = (AllowPrivateAccess = "true"))
	uint8 GroggyCount;

	/** 그로기에서 회복 후의 체력량, 회복 후의 체력량은 MaxHealth * RecoveryHealthPercentage로 설정된다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Groggy", meta = (AllowPrivateAccess = "true", ClampMin = "0.01", ClampMax = "1.0"))
	float RecoveryHealthPercentage;

	/** 그로기에서 사망 전이 Timer */
	FTimerHandle GroggyTimer;

	/** 그로기 상태에서의 LookSensitivity. Groggy 상태에 진입할 때마다 LookSensitivity를 이 값으로 설정한다. */
	UPROPERTY(EditDefaultsOnly, Category = "Character|Groggy")
	float GroggyLookSensitivity;

	/** 그로기 상태에서 부활할 때 Hold해야 하는 시간. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Groggy", meta = (AllowPrivateAccess = "true"))
	float RescueRequireTime;

	/** 현재 전투 중인지 여부 */
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	uint8 bIsInCombat : 1;
	
	/** 현재 캐릭터를 타겟팅하고 있는 Actor의 개수. */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int TargetingActorCount;

	// Tick을 썼다면 쉽게 관리했겠지만 현재로는 Timer를 사용해서 관리한다.
	/** 체력 회복 중지 상태에서 체력 회복을 시작하기 위해 필요한 시간 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Combat", meta = (AllowPrivateAccess = "true"))
	float HealthRegenDelay;

	/** 1초당 체력 회복 비율이다. MaxHealth * HealthRegenRate 만큼 회복한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Combat", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float HealthRegenRate;
	
	/** 전투 종료 시에 체력 회복을 위한 타이머 핸들 */
	FTimerHandle HealthRegenStartTimer;
	
	// Gather와 같은 정보는 추후 다른 곳으로 이동될 수 있지만 일단은 캐릭터에 구현한다.

	/** 채광 속도. 2.0일 경우 2배의 속도로 채광한다. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category= "Character|Stat", meta = (AllowPrivateAccess = "true"))
	float GatherMultiplier;

	/** 디버그 카메라 활성화 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Debug, meta = (AllowPrivateAccess = "true"))
	uint8 bUseDebugCamera : 1;

	/** 캐릭터의 현재 상태. 지상 혹은 수중 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	EEnvironmentState EnvironmentState;

	/** Capture 상태 여부. 추후 상태가 많아지면 상태 패턴 이용을 고려 */
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	uint8 bIsCaptured : 1;

	/** Capture 상태에서 Fade Out / In 되는 시간 */
	UPROPERTY(EditAnywhere, Category = Character, meta = (AllowPrivateAccess = "true"))
	float CaptureFadeTime;

	/** 피격 시의 출혈 효과를 내는 Noise System Power */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	float BloodEmitPower;

	/** 캐릭터가 수풀에 숨어있는지 여부 */
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	uint8 bIsHideInSeaweed : 1;

	/** 초과 적재 기준 무게. 초과할 경우 속도를 감소시킨다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Weight", meta = (AllowPrivateAccess = "true"))
	float OverloadWeight;

	/** 초과 적재 시의 속도 감소 비율. [0, 1]의 범위로 속도를 감소시킨다. 0.4일 경우 40% 감소해서 60%의 속도이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Weight", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float OverloadSpeedFactor;

	/** 캐릭터의 효과가 적용된 최종 속도 */
	UPROPERTY(Transient, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	float EffectiveSpeed;

	/** 캐릭터가 감속할 수 있는 최소 속도 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	float MinSpeed;

	// @ToDo: Multiplier를 통합 적용
	// @ToDo: DPV 상황 추가
	
	/** Sprint 시에 적용되는 속도 배율. Sprint가 적용되면 EffectiveSpeed에 곱해진다. */
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	float SprintMultiplier;

	/** 특정 Zone에서 적용되는 속도 배율. Zone에 따라 다르게 적용된다. */
	UPROPERTY(BlueprintReadWrite, Category = Character, meta = (AllowPrivateAccess = "true"))
	float ZoneSpeedMultiplier;

	/** 생성할 레이더 BP */
	UPROPERTY(EditDefaultsOnly, Category = "Character|Radar", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ARadar> RadarClass;

	/** 생성한 레이더 인스턴스 */
	UPROPERTY()
	TObjectPtr<class ARadar> RadarObject;

	/** 이름 표기 위젯 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UNameWidgetComponent> NameWidgetComponent;

	/** 레이더가 생성된 위치 오프셋. 카메라 기준으로 부착이 된다. */
	UPROPERTY(EditDefaultsOnly, Category = "Character|Radar", meta = (AllowPrivateAccess = "true"))
	FVector RadarOffset;

	/** 레이더가 생성된 회전 오프셋. 카메라 기준으로 부착이 된다. */
	UPROPERTY(EditAnywhere, Category = "Character|Radar", meta = (AllowPrivateAccess = "true"))
	FRotator RadarRotation;

	/** 레이더 활성화 여부 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing=OnRep_bIsRadarOn, Category = Character, meta = (AllowPrivateAccess = "true"))
	uint8 bIsRadarOn : 1;

	/** Animation를 재생하기 위한 정보이다. 애니메이션 재생 시에 한 번에 동기화되기 위해 사용된다. */
	UPROPERTY(BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	FAnimSyncState AnimSyncState;

	/** Animation 1P Montage 적용 중인지 여부 */
	uint8 bIsAnim1PSyncStateOverride : 1;

	/** Animation 3P Montage 적용 중인지 여부 */
	uint8 bIsAnim3PSyncStateOverride : 1;
	
	/** Montage 재생 중에 적용될 Anim Sync State */
	FAnimSyncState OverrideAnimSyncState;

	/** 이동 입력, 3차원 입력을 받는다. 캐릭터의 XYZ 축대로 맵핑을 한다. Forward : X, Right : Y, Up : Z */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	/** 점프 입력. 지상에서만 작동한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> JumpAction;

	/** 스프린트 입력. 누르고 있는 동안 Sprint가 활성화된다. Sprint가 모자를 경우 활성화가 안 되므로 다시 떼었다가 눌러야 한다. */
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

	/** 재장전 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ReloadAction;

	/** 1번 슬롯 장착 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> EquipSlot1Action;

	/** 2번 슬롯 장착 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> EquipSlot2Action;

	/** 3번 슬롯 장착 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> EquipSlot3Action;

	/** 감정 표현 1번 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> EmoteAction1;

	/** 감정 표현 2번 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> EmoteAction2;

	/** 감정 표현 3번 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> EmoteAction3;

	/** 게임에 사용될 1인칭 Camera Component의 Spring Arm. 회전 Smoothing을 위해 사용한다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> FirstPersonCameraArm;
	
	/** 게임에 사용될 1인칭 Camera Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> FirstPersonCameraComponent;

	/** 디버깅 목적으로 사용할 3인칭 Camera Component의 Spring Arm */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> CameraBoom;

	/** 디버깅 목적으로 사용될 3인칭 Camera Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> ThirdPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> Mesh1PSpringArm;
	
	/** 게임에 사용될 1인칭 Mesh Component */
	UPROPERTY(VisibleAnywhere, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USkeletalMeshComponent> Mesh1P;

	/** 캐릭터의 산소 상태를 관리하는 Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UOxygenComponent> OxygenComponent;

	/** 캐릭터의 스태미너 상태를 관리하는 Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaminaComponent> StaminaComponent;

	/** 캐릭터의 실드를 관리하는 Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UShieldComponent> ShieldComponent;

	/** 캐릭터 출혈을 시뮬레이션을 하는 Noise Emitter Component */
	UPROPERTY()
	TObjectPtr<class UPawnNoiseEmitterComponent> NoiseEmitterComponent;

	/** 상호작용 실행하게 하는 Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UADInteractionComponent> InteractionComponent;

	/** Shop의 Interaction을 실행할 Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UShopInteractionComponent> ShopInteractionComponent;

	/** 장착 아이템 효과 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UEquipUseComponent> EquipUseComponent;

	/** Lantern 컴포넌트. 캐릭터가 라이트를 켜고 끌 수 있다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULanternComponent> LanternComponent;

	/** 전투 효과 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCombatEffectComponent> CombatEffectComponent;
	
	/** 수중 효과 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UUnderwaterEffectComponent> UnderwaterEffectComponent;
	
	/** 발자국 소리 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UFootstepComponent> FootstepComponent;

	/** 상호작용 대상이 되게 하는 컴포넌트 */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UADInteractableComponent> InteractableComponent;

	/** 래그돌 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class URagdollReplicationComponent> RagdollComponent;

	/** 인벤토리 컴포넌트 캐시 */
	UPROPERTY()
	TObjectPtr<class UADInventoryComponent> CachedInventoryComponent;

	/** 장착 아이템 렌더링을 위한 컴포넌트 */
	UPROPERTY()
	TObjectPtr<class UEquipRenderComponent> EquipRenderComp;

	UPROPERTY(EditAnywhere, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UHoldInteractionWidget> HoldWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UI", meta = (AllowPrivateAccess = "true"))
	class UHoldInteractionWidget* HoldWidgetInstance;

	/** Tool 소켓 명 (1P/3P 공용) */
	FName LaserSocketName = TEXT("Laser");

	/** 감정 표현 몽타주 배열. 순서대로 Emote1, Emote2, Emote3에 해당한다. */
	UPROPERTY(EditDefaultsOnly, Category = "Character|Emote", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UAnimMontage>> EmoteAnimationMontages;

	/** 현재 상호 작용 택스트 */
	FString InteractionDescription;

	/** 현재 상호 작용이 Hold 모드인지 여부 */
	bool bIsInteractionHoldMode;

	/** 그로기 상태 상호 작용 택스트. 구조 상황에 출력 */
	UPROPERTY(EditDefaultsOnly, Category= "Character|Interaction", meta = (AllowPrivateAccess = "true"))
	FString GroggyInteractionDescription;

	/** 사망 상태 상호 작용 택스트. 시체 들기 상황에 출력 */
	UPROPERTY(EditDefaultsOnly, Category= "Character|Interaction", meta = (AllowPrivateAccess = "true"))
	FString DeathInteractionDescription;

	/** 사망 상태에서 시체 들은 상황에서 출력 */
	UPROPERTY(EditDefaultsOnly, Category= "Character|Interaction", meta = (AllowPrivateAccess = "true"))
	FString DeathGrabReleaseDescription;
	
#pragma endregion

#pragma region Getter Setter

public:
	
	/** 캐릭터의 Oxygen Component를 반환 */
	FORCEINLINE class UOxygenComponent* GetOxygenComponent() const { return OxygenComponent; }

	/** 캐릭터의 Shield Component를 반환 */
	FORCEINLINE class UShieldComponent* GetShieldComponent() const { return ShieldComponent; }

	/** Interaction Component를 반환 */
	FORCEINLINE UADInteractionComponent* GetInteractionComponent() const { return InteractionComponent; }

	/** Shop Interaction Component를 반환 */
	FORCEINLINE UShopInteractionComponent* GetShopInteractionComponent() const { return ShopInteractionComponent; }

	/** 1인칭 Camera Component를 반환 */
	FORCEINLINE UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	/** 1인칭 Camera Arm을 반환 */
	FORCEINLINE USpringArmComponent* GetMesh1PSpringArm() const { return Mesh1PSpringArm; }

	/** 캐릭터의 상태를 반환 */
	FORCEINLINE EEnvironmentState GetEnvironmentState() const { return EnvironmentState; }

	/** 현재 Locomotion Mode를 반환 */
	FORCEINLINE ELocomotionMode GetLocomotionMode() const { return LocomotionMode; }

	/** 장착 아이템 컴포넌트 반환 */
	FORCEINLINE UEquipUseComponent* GetEquipUseComponent() const { return EquipUseComponent; }

	/** 장착 아이템 렌더링 컴포넌트 반환 */
	FORCEINLINE UEquipRenderComponent* GetEquipRenderComponent() const { return EquipRenderComp; }

	/** 캐릭터의 현재 상태를 반환 */
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }

	/** 캐릭터 무적 상태를 반환. 현재는 Server에서만 유효하다. */
	FORCEINLINE bool IsInvincible() const { return bIsInvincible; }

	/** 캐릭터 무적 상태를 설정. 현재는 Server에서만 유효하다. */
	FORCEINLINE void SetInvincible(const bool bNewInvincible) { bIsInvincible = bNewInvincible; }

	/** 캐릭터가 일반 상태인지 여부를 반환 */
	FORCEINLINE bool IsNormal() const { return CharacterState == ECharacterState::Normal; }

	/** 캐릭터가 Groggy 상태인지 여부를 반환 */
	FORCEINLINE bool IsGroggy() const { return CharacterState == ECharacterState::Groggy; }

	/** 캐릭터가 Death 상태인지 여부를 반환 */
	FORCEINLINE bool IsDeath() const { return CharacterState == ECharacterState::Death; }

	/** 캐릭터가 현재 살아있는지 여부를 반환. 살아 있으면 타겟팅될 수 있다. */
	FORCEINLINE bool IsAlive() const;

	/** 캐릭터의 남은 그로기 시간을 반환 */
	UFUNCTION(BlueprintCallable)
	float GetRemainGroggyTime() const;

	/** 현재 캐릭터의 최종 속도를 반환 */
	FORCEINLINE float GetEffectiveSpeed() const { return EffectiveSpeed; }

	/** 현재 캐릭터가 달리기 상태인지를 반환 */
	FORCEINLINE bool IsSprinting() const;

	/** 현재 캐릭터가 수중 상태인지 여부를 반환 */
	FORCEINLINE bool IsHideInSeaweed() const { return bIsHideInSeaweed; }

	/** 현재 캐릭터가 수풀에 숨어있는지 여부를 설정 */
	void SetHideInSeaweed(const bool bNewHideInSeaweed);

	/** 초과 무게 상태인지를 반환. 무게 >= 최대 무게일 때 True를 반환 */
	UFUNCTION(BlueprintCallable)
	bool IsOverloaded() const;

	/** 채광 속도를 반환 2.0일 경우 2배로 빠르게 채광한다. */
	FORCEINLINE float GetGatherMultiplier() const { return GatherMultiplier; }

	/** Mesh 1P 메시를 반환 */
	FORCEINLINE USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }

	/** 1인칭 메시 Strafe 여부 반환 */
	FORCEINLINE bool Is1PStrafe() const {return bIsAnim1PSyncStateOverride ? AnimSyncState.bIsStrafing : OverrideAnimSyncState.bIsStrafing;}

	/** 3인칭 메시 Strafe 여부 반환 */
	FORCEINLINE bool Is3PStrafe() const {return bIsAnim3PSyncStateOverride ? AnimSyncState.bIsStrafing : OverrideAnimSyncState.bIsStrafing;}
	
	/** 스프린트 적용 속도 반환 */
	FORCEINLINE float GetSprintSpeed() const { return StatComponent->MoveSpeed * SprintMultiplier; }

	/** Zone Speed Multiplier를 반환 */
	FORCEINLINE float GetZoneSpeedMultiplier() const { return ZoneSpeedMultiplier; }

	/** Zone Speed Multiplier를 설정 */
	void SetZoneSpeedMultiplier(float NewMultiplier);
	
	/** 현재 캐릭터가 무기를 장착하고 있는지 여부를 반환 */
	bool IsWeaponEquipped() const;

	/** 현재 캐릭터 전투 중인지 여부를 반환. 현재는 Server에서만 작동 */
	FORCEINLINE bool IsInCombat() const { return bIsInCombat; }

	/** 현재 생성된 실드 히트 위젯을 반환 */
	UUserWidget* GetShieldHitWidget() const;
	
#pragma endregion
};
