// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "UnitBase.h"
#include "Interface/IADInteractable.h"
#include "AbyssDiverUnderWorld.h"
#include "LocomotionMode.h"
#include "StatComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "UnderwaterCharacter.generated.h"

#if UE_BUILD_SHIPPING
enum class EMoveDirection : uint8;
	#define LOG_ABYSS_DIVER_COMPILE_VERBOSITY Error
#else
	#define LOG_ABYSS_DIVER_COMPILE_VERBOSITY All
#endif

#define LOG_NETWORK(Category, Verbosity, Format, ...) \
	UE_LOG(Category, Verbosity, TEXT("[%s] %s %s"), LOG_NETMODEINFO, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))

enum EDepthZone : uint8;
DECLARE_LOG_CATEGORY_EXTERN(LogAbyssDiverCharacter, Log, LOG_ABYSS_DIVER_COMPILE_VERBOSITY);

// @TODO : Character Status Replicate 문제
// 1. 최대 Stamina는 최대 Oxygen에 비례하는데 값이 따로따로 Replicate될 수 있다. 이렇게 되면 항상 오차가 존재하게 된다.
// 2. 매 프레임 변수가 Replicate 되는 상황이 발생한다. 4인 유저 체제일 경우 어느정도 대역폭을 소모하게 될 지 모른다.
// 실제 게임 플레이 태스트와 프로파일링을 통해서 문제를 해결해야 한다.
// 3. Stamina, Oxygen 컴포넌트가 분리되어서 더 복잡해지고 있는 상황일 수 있다. 추후 구현이 필요 이상으로 복잡해지면 합치는 것을 고려한다.

enum class ESFX : uint8;
enum class ELocomotionMode : uint8;

/* 캐릭터의 지상, 수중을 결정, Move 로직, Animation이 변경되고 사용 가능 기능이 제한된다. */
UENUM(BlueprintType)
enum class EEnvironmentState : uint8
{
	Underwater,
	Ground,
	MAX UMETA(Hidden)
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
class URadar2DComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AUnderwaterCharacter : public AUnitBase, public IIADInteractable
{
	GENERATED_BODY()

public:
	AUnderwaterCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void PostInitializeComponents() override;
	virtual void PostNetInit() override;
	virtual void OnRep_PlayerState() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;
	virtual void Destroyed() override;

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
	virtual float GetHoldDuration_Implementation(AActor* InstigatorActor) const override;
	
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

	/** 캐릭터를 사망시킨다. Authority Node에서만 실행되어야 한다. */
	UFUNCTION(BlueprintCallable)
	void Kill();

	/** 부활 상태 초기화 설정을 한다. Possess 이후에 호출할 것 */
	void Respawn();
	
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
	void OnTargeted(AActor* TargetingActor);

	/** Monster에 의해 UnTarget 되었을 때 호출된다. Authority Node에서만 유효하다. */
	UFUNCTION(BlueprintCallable)
	void OnUntargeted(AActor* TargetingActor);

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

	/*
	 * Bind Character ----- Rope ----- Bound Character 
	 * Bind Character : 로프를 묶은 캐릭터, Bound Character를 여러개 묶을 수 있다.
	 * Bound Character : 로프에 묶인 캐릭터
	 * Bound Character 중심으로 로직을 작성
	 * Bind Character, Bound Character를 Replicate해서 구현
	 * Cable Binding Actor : 로프를 시각화하고 물리 제약을 적용하는 Actor
	 * Cable Binding Actor는 Bound Character가 소유
	 */

	/** Bind Character가 Binder Character를 로프에 묶이는 요청을 한다. Authority Node에서만 실행되어야 한다. */
	void RequestBind(AUnderwaterCharacter* RequestBinderCharacter);

	/** Bound Character 함수. 현재 캐릭터를 UnBind 한다. Binder가 시체를 들고 있을 수 없는 상황에서도 호출된다.
	 * UnBind 함수는 Binder에서 BoundCharacter Array를 수정하므로 Binder Character에서 루프를 순회하면서 UnBind를 호출하면 문제가 생긴다.
	 * GetBoundCharacters는 복사본을 반환하므로 안전하게 순회가 가능하다.
	 */
	void UnBind();

	/** Bind Character 함수. 현재 캐릭터를 UnBind 한다. Binder가 시체를 들고 있는 상황에서 호출된다. */
	void UnbindAllBoundCharacters();
	
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

	UFUNCTION()
	void OnRep_InCombat();

	/** 관전을 당할 때 호출되는 함수 */
	void OnSpectated();

	/** 관전 당하는 것이 끝났을 때 호출되는 함수 */
	void OnEndSpectated();

	/** 감정 표현 재생 요청
	 * EmoteIndex : 0 ~ n - 1 (EmoteAnimationMontages 배열의 인덱스)
	 * Emote 재생이 가능할 경우에만 재생.
	 */
	UFUNCTION(BlueprintCallable)
	void RequestPlayEmote(int32 EmoteIndex);

	/** Name Widget 가시성 설정
	 * false일 경우 항상 숨김 처리가 된다.
	 * true일 경우 캐릭터 정책에 따라서 Name Widget의 가시성이 결정된다.
	 * Locally Controlled Character의 Name Widget은 항상 숨김 처리된다.
	 * Spectated Character의 Name Widget은 항상 숨김 처리된다.
	 */
	void SetNameWidgetEnabled(bool bNewVisibility);

	/** 새로운 위치로 순간이동한다. 순간 이동 후에는 속도를 초기화하며 View Rotation을 설정한다. */
	void Teleport(const FVector& NewLocation, const FRotator& ViewRotation);
	
protected:

	/** 초기 물리 볼륨이 설정됬을 때 호출된다. BeginPlay 다음 프레임에 호출된다. */
	void OnInitialPhysicsVolumeSet();
	
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

	/** Groggy 상태에서 사망까지 걸리는 시간을 계산한다. 사망을 할 수록 기간이 길어진다.
	 * Groggy Time은 [,0.001]으로 0이상이 되는 것을 보장한다.
	 */
	virtual float CalculateGroggyTime(float CurrentGroggyDuration, uint8 CalculateGroggyCount) const;
	
	/** 캐릭터 사망 시에 Blueprint에서 호출될 함수 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnDeath"))
	void K2_OnDeath();

	/** 캐릭터의 환경이 변경됬을 시에 Blueprint에서 호출될 함수 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnEnvironmentStateChanged"))
	void K2_OnEnvironmentStateChanged(EEnvironmentState OldEnvironmentState, EEnvironmentState NewEnvironmentState);

	/** Player State를 기반으로 Player Status 초기화 */
	void InitPlayerStatus(class AADPlayerState* ADPlayerState);

	/** Upgrade Component의 정보를 바탕으로 초기화 */
	void ApplyUpgradeFactor(class UUpgradeComponent* UpgradeComponent);
	
	/** Capture State Multicast
	 * Owner : 암전 효과, 입력 처리
	 * All : Mesh 비활성화
	 */
	UFUNCTION(NetMulticast, Reliable)
	void M_StartCaptureState();
	void M_StartCaptureState_Implementation();

	/** Stop Capture 시의 Camera 효과를 재생
	 * - Normal 로 복귀 시에는 페이드 아웃 효과
	 * - Groggy 로 복귀 시에는 Groggy Timer만큼 Camera Fade In 후에 Groggy Fade Out 효과
	 * - Death 시에는 암전 유지
	 */
	void PlayStopCaptureCameraEffect();

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

	/** 캐릭터가 사망을 완료했을 때 호출되는 함수. 관전으로 변경된다. */
	void EndDeath();

	/** 현재 소지 중인 교환 가능 아이템들을 모두 드랍한다. */
	void DropAllExchangeableItems();

	/** 현재 캐릭터의 이동 속도를 계산한다. */
	float CalculateEffectiveSpeed() const;

	/** 현재 상태 속도 갱신.(무게, Sprint) */
	UFUNCTION()
	void AdjustSpeed();

	/** 1인칭, 3인칭 시점의 메시를 설정 */
	void SetMeshFirstPersonSetting(bool bIsFirstPerson);

	/** Pawn을 떠날 때 호출되는 함수. Pawn이 떠나면 캐릭터는 관전 상태로 변경된다. */
	void OnLeavePawn();

	/** 현재 상태에 맞춰서 Blur 효과를 업데이트한다. */
	void UpdateBlurEffect();
	
	/** Blur 효과 적용 여부를 설정한다. */
	void SetBlurEffect(const bool bEnable);

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
	void MoveUnderwater(const FVector& MoveInput);

	/** 지상 이동 함수 */
	void MoveGround(const FVector& MoveInput);

	/** 이동 방향을 업데이트 한다. 현재 방향이 변경되었을 경우 true를 반환하고 아닐 경우 false를 반환한다. */
	bool UpdateMoveDirection(const FVector& MoveInput);

	/** 현재 이동 방향을 Server에 보고한다. */
	UFUNCTION(Server, Reliable)
	void S_ReportMoveDirection(EMoveDirection NewMoveDirection);
	void S_ReportMoveDirection_Implementation(EMoveDirection NewMoveDirection);
	
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


	/** Server에 감정 표현 재생 요청 */
	UFUNCTION(Server, Reliable)
	void S_PlayEmote(uint8 EmoteIndex);
	void S_PlayEmote_Implementation(uint8 EmoteIndex);

	/** 감정 표현 재생 Multicast 전파 */
	UFUNCTION(NetMulticast, Reliable)
	void M_BroadcastPlayEmote(int8 EmoteIndex);
	void M_BroadcastPlayEmote_Implementation(int8 EmoteIndex);

	/** 감정 표현 재생 가능 여부를 반환 */
	bool CanPlayEmote() const;

	/** 감정 표현 몽타주를 반환. Index: 0~n-1 */
	UAnimMontage* GetEmoteMontage(int8 EmoteIndex) const;
	
	/** 감정 표현을 중지 */
	void RequestStopPlayingEmote(int8 EmoteIndex);

	/** 감정 표현 중지 Multicast 전파 */
	UFUNCTION(NetMulticast, Reliable)
	void M_BroadcastStopPlyingEmote(int8 EmoteIndex);
	void M_BroadcastStopPlyingEmote_Implementation(int8 EmoteIndex);

	/** 감정 표현 중지 Server RPC */
	UFUNCTION(Server, Reliable)
	void S_StopPlayingEmote(int8 EmoteIndex);
	void S_StopPlayingEmote_Implementation(int8 EmoteIndex);
	
	/** 감정 표현 몽타주가 끝났을 때 호출되는 함수 */
	UFUNCTION()
	void OnEmoteEnd(UAnimMontage* AnimMontage, bool bInterupted);

	/** 카메라 모드를 전환한다. 1인칭, 감정 표현을 위한 3인칭 모드로 전환한다.
	 * PreCondition : 1인칭 카메라 Transition이 완료되어야 한다. Transition 도중에 시작하는 경우는 없다.
	 */
	void StartEmoteCameraTransition();

	/** Mesh Visibility를 카메라 모드에 맞춰서 설정한다. */
	void SetCameraFirstPerson(bool bFirstPersonCamera);
	
	/** 카메라 Transition Update */
	void UpdateCameraTransition();

	/** Binder Character 함수. Bound Characters를 저장한다. */
	void BindToCharacter(AUnderwaterCharacter* BoundCharacter);

	/** Binder Character 함수 */
	void UnbindToCharacter(AUnderwaterCharacter* BoundCharacter);
	
	/** Bound Character 함수. Binder Character와 로프를 연결한다. */
	void ConnectRope(AUnderwaterCharacter* BinderCharacter);

	/** Bound Character 함수. 로프를 해제한다. */
	void DisconnectRope();

	/** Bound Character 함수. Machine 기준으로 현재의 Interactable을 설정한다.
	 * Binder Character가 Machine의 Local이면 Interactable을 활성화하고 해제 기능을 활성화
	 * Binder Character가 Machine의 Local이 아니면 Interactable을 비활성화하고 해제 기능을 비활성화
	 */
	void UpdateBindInteractable();

	/** Bound Character 함수. BindCharacter의 Replicate 함수. Bound Characters는 Bind Character에서 갱신되므로 Bind Character의 기능만 구현하면 된다. */
	UFUNCTION()
	void OnRep_BindCharacter();

	/** Binder Character 함수. BoundCharacters의 Replicate 함수. 들고 있는 캐릭터에 따라서 감속 처리 */
	UFUNCTION()
	void OnRep_BoundCharacters();

	/** TargetingActors의 유효성을 검증한다. */
	int32 ValidateTargetingActors();

	/** TargetingActors의 유효하지 않은 Actor를 제거한다. */
	void CleanupTargetingActors();

	/** DepthZone이 변경될 때 호출되는 함수 */
	UFUNCTION()
	void OnDepthZoneChanged(EDepthZone OldDepthZone, EDepthZone NewDepthZone);

	/** 현재 DepthZone에 따른 산소 소비율을 반환한다. */
	float GetOxygenConsumeRate(EDepthZone DepthZone) const;

	// 현재는 Debug 기능을 위해서 간략하게 구현되었다.
	// 추후에 유저 조작으로 NameWidget을 숨김처리하거나 하는 기능이 추가되면
	// Name Widget에서 Callback 을 통해서 구현한다.
	// 현재 방식상으로는 호출 비용이 크다.
	
	/** 현재 상태에 따라 Name Widget의 가시성을 설정한다.
	 * Local Player가 아니고 PC에서 Name Widget 가시성이 참일 경우에만 Name Widget이 보인다.
	 */
	void InitNameWidgetEnabled();

	/** Teleport 후에 Client의 Control Rotation을 설정한다. */
	UFUNCTION(Reliable, Client)
	void C_ApplyControlRotation(const FRotator& NewControlRotation);
	void C_ApplyControlRotation_Implementation(const FRotator& NewControlRotation);
	
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

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCaptureStart);
	/** 캐릭터가 캡쳐 상태에 진입했을 때 호출되는 델리게이트. 모든 노드에서 실행 */
	UPROPERTY(BlueprintAssignable)
	FOnCaptureStart OnCaptureStartDelegate;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCaptureEnd);
	/** 캐릭터가 캡쳐 상태에서 벗어났을 때 호출되는 델리게이트. 모든 노드에서 실행 */
	UPROPERTY(BlueprintAssignable)
	FOnCaptureEnd OnCaptureEndDelegate;
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGroggy);
	/** 캐릭터가 그로기 상태에 진입했을 때 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable)
	FOnGroggy OnGroggyDelegate;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCharacterStateChanged, AUnderwaterCharacter*, Character, ECharacterState, OldCharacterState, ECharacterState, NewCharacterState);
	/** 캐릭터 상태가 변경되었을 때 호출되는 델리게이트, 상태에 진입했을 경우 Capture 상태일 수도 있는 것을 확인해야 한다. */
	UPROPERTY(BlueprintAssignable)
	FOnCharacterStateChanged OnCharacterStateChangedDelegate;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnvironmentStateChanged, EEnvironmentState, OldEnvironmentState, EEnvironmentState, NewEnvironmentState);
	/** 캐릭터의 환경 상태가 변경되었을 때 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable)
	FOnEnvironmentStateChanged OnEnvironmentStateChangedDelegate;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageTaken, float, DamageAmount, float, CurrentHealth);
	/** 캐릭터가 피해를 입었을 때 호출되는 델리게이트, DamageAmount = Health Damage Taken + Shield Damage Taken.
	 * 체력 계산, 상태 전이가 모두 완료된 뒤에 호출된다.
	 * Normal 상태일 때만 호출된다. 실드에만 데미지가 들어갔을 경우 호출되지 않는다.
	 */
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

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEmoteStart);
	/** 감정 표현 시작 시 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Character|Emote")
	FOnEmoteStart OnEmoteStartDelegate;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEmoteEnd);
	/** 감정 표현 종료 시 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Character|Emote")
	FOnEmoteStart OnEmoteEndDelegate;

	DECLARE_MULTICAST_DELEGATE(FOnGroggyRevive);
	/** 그로기에 걸린 팀원 부활 시 호출되는 델리게이트 */
	FOnGroggyRevive OnGroggyReviveDelegate;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnHiddenChanged, bool /* bNewHidden */);
	/** 캐릭터의 Hidden 상태가 변경되었을 때 호출되는 델리게이트 */
	FOnHiddenChanged OnHiddenChangedDelegate;

	UPROPERTY(VisibleAnywhere, Category = "Mining")
	/** 현재 1p에 장착된 Tool 인스턴스 */
	TObjectPtr<AActor> SpawnedTool;

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

	uint8 bIsMining : 1 = false;

private:

	UPROPERTY()
	class AADPlayerController* OwnerController;
	
	/** 현재 캐릭터를 Possess한 PlayerController의 Player Index */
	int8 PlayerIndex;
	
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

	/** 감정 표현 여부. 감정 표현을 실행하면 True가 되고 False가 되는 시점은 First Person Camera로의 Transition이 종료됬을 때이다. */
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	uint8 bPlayingEmote : 1;

	/** 현재 재생 중인 감정 표현 인덱스 */
	int8 PlayEmoteIndex;

	/** Camera Transition 시에 Timer Update 함수 Interval */
	UPROPERTY(EditDefaultsOnly, Category = "Character|Emote", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float CameraTransitionUpdateInterval;

	/** 카메라 Transition Alpha 방향. 1.0f이면 Emote Camera로 이동, -1.0f이면 First Person Camera로 이동한다.
	 * 카메라 Transition이 시작되면 1.0f가 되고 애니메이션이 종료되거나 취소되면 -1.0f가 된다. */
	float CameraTransitionDirection;
	
	/** 카메라 Transition 시에 경과 시간. Alpha = CameraTransitionTimeElapsed / CameraTransitionDuration */
	float CameraTransitionTimeElapsed;

	/** 카메라 Transition에 걸리는 시간 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|Emote", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float CameraTransitionDuration;

	/** Emote Camera 시에 Spring Arm 길이 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|Emote", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float EmoteCameraTransitionLength;

	/** Emote Camera Transition 시에 Spring Arm Easing Type */
	UPROPERTY(EditDefaultsOnly, Category = "Character|Emote", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<EEasingFunc::Type> EmoteCameraTransitionEasingType;
	
	/** 감정 표현 몽타주 배열. 순서대로 Emote1, Emote2, Emote3에 해당한다. */
	UPROPERTY(EditDefaultsOnly, Category = "Character|Emote", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UAnimMontage>> EmoteAnimationMontages;

	/** 감정 표현 중에 3인칭 카메라 전환을 위한 Timer */
	FTimerHandle EmoteCameraTransitionTimer;
	
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

	/** Fade 된 상태에서 Normal 상태로 전이됬을 때 Camera Fade In 되는 시간 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|Normal", meta = (AllowPrivateAccess = "true"))
	float NormalStateFadeInDuration;
	
	/** 그로기 상태에서 사망까지 걸리는 시간. 그로기 종료 시에 Groggy Count에 따라서 결정되고 다음 Groggy 시에 적용된다.
	 * 현재 그로기 시간을 알고 싶으면 GetRemainGroggyTime을 호출 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Groggy", meta = (AllowPrivateAccess = "true"))
	float GroggyDuration;

	/** 그로기 상태에서 공격 받았을 경우 줄어드는 Groggy 시간. 초 단위이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Groggy", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float GroggyHitPenalty;

	/** 그로기 상태에 진입될 때마다 감소하는 Groggy Duration 비율. [0, 1]의 범위로 설정한다. 0.3일 경우 0.7배로 감소한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Groggy", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float GroggyReductionRate;

	/** 그로기 상태를 계산할 때 최소 그로기 상태 시간. GroggyDuration * GroggyReductionRate보다 작을 경우 이 값으로 설정한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Groggy", meta = (AllowPrivateAccess = "true", ClampMin = "0.001"))
	float MinGroggyDuration;

	/** 그로기 상태에 진입한 횟수. uint8이라서 오버플로우에 주의 */
	UPROPERTY(BlueprintReadOnly, Category = "Character|Groggy", meta = (AllowPrivateAccess = "true"))
	uint8 GroggyCount;

	/** 그로기에서 회복 후의 체력량, 회복 후의 체력량은 MaxHealth * RecoveryHealthPercentage로 설정된다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Groggy", meta = (AllowPrivateAccess = "true", ClampMin = "0.01", ClampMax = "1.0"))
	float RecoveryHealthPercentage;

	/** 그로기에서 회복 후의 산소량, 회복 후의 산소량은 MaxOxygen * RecoveryOxygenPercentage로 설정된다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Groggy", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float RecoveryOxygenPenaltyRate;

	/** 그로기에서 사망 전이 Timer */
	FTimerHandle GroggyTimer;

	/** Capture State에서 탈출 후에 그로기 카메라 페이드 인 Timer */
	FTimerHandle GroggyCameraFadeTimer;

	/** 그로기 상태에서의 LookSensitivity. Groggy 상태에 진입할 때마다 LookSensitivity를 이 값으로 설정한다. */
	UPROPERTY(EditDefaultsOnly, Category = "Character|Groggy")
	float GroggyLookSensitivity;

	/** 그로기 효과음 */
	ESFX GroggySfx;

	/** 그로기 효과음 사운드 ID */
	int32 GroggySfxId;

	/** 그로기 상태에서 부활할 때 Hold해야 하는 시간. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Groggy", meta = (AllowPrivateAccess = "true"))
	float RescueRequireTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Death", meta = (AllowPrivateAccess = "true"))
	ESFX ResurrectSFX;
	
	/** 현재 전투 중인지 여부 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_InCombat, Category = Character, meta = (AllowPrivateAccess = "true"))
	uint8 bIsInCombat : 1;
	
	/** 현재 캐릭터를 타겟팅하고 있는 Actor들의 집합 */
	TSet<TWeakObjectPtr<AActor>> TargetingActors;

	/** True일 경우 주기적으로 Targeting Actors의 유효성을 확인한다. */
	uint8 bAutoStartCleanupTargetingActors : 1;

	/** TargetingActors의 유효성을 검사하는 Timer 핸들 */
	FTimerHandle TargetingActorsCleanupTimer;

	/** TargetingActors의 유효성을 검사하는 주기. 초 단위로 설정한다. */
	float TargetingActorsCleanupInterval;

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

	/** State를 변환해야 하나, Capture State 이기 때문에 Pending된 상태 */
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	uint8 bPendingDeathAfterCapture : 1 = 0;

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

	/** 후방 이동 시의 속도 비율. 0.7일 경우 70%의 속도로 이동한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Move", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float BackwardMoveSpeedFactor = 0.7f;
	
	/** 캐릭터의 효과가 적용된 최종 속도 */
	UPROPERTY(Transient, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	float EffectiveSpeed;

	/** 캐릭터가 감속할 수 있는 최소 속도 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	float MinSpeed;

	/** 지상에서 기본 속도. 지상은 업그레이드 영향을 받지 않는다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	float BaseGroundSpeed;

	/** Upgrade가 적용된 최종 속도. Ground, Water을 전환할 때 속도를 갱신하기 위해 속도를 저장한다. 초기값은 StatComponent의 값을 참조한다. */
	float BaseSwimSpeed;

	/** 캐릭터의 현재 이동 방향 */
	EMoveDirection MoveDirection = EMoveDirection::Other;

	/** Upgrade 추가 이동 속도 */ 
	float UpgradeSwimSpeed = 0.0f;

	// @ToDo: Multiplier를 통합 적용
	// @ToDo: DPV 상황 추가
	
	/** Sprint 시에 적용되는 속도 배율. Sprint가 적용되면 EffectiveSpeed에 곱해진다. */
	UPROPERTY(EditDefaultsOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	float SprintMultiplier;

	/** 특정 Zone에서 적용되는 속도 배율. Zone에 따라 다르게 적용된다. */
	UPROPERTY(BlueprintReadWrite, Category = Character, meta = (AllowPrivateAccess = "true"))
	float ZoneSpeedMultiplier;

	/** 이름 표기 위젯 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UNameWidgetComponent> NameWidgetComponent;

	/** 깊이 위젯 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UDepthComponent> DepthComponent;

	/** 안전 구역에서 산소 소모량 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float SafeZoneOxygenConsumeRate;
	
	/** 경고 구역에서 산소 소모량 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float WarningZoneOxygenConsumeRate;

	/** 위험 구역에서 산소 소모량 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float DangerZoneOxygenConsumeRate;

	/** 레이더 활성화 여부 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = Character, meta = (AllowPrivateAccess = "true"))
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

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<URadar2DComponent> RadarComponent;

	/** Tool 소켓 명 (1P/3P 공용) */
	FName LaserSocketName = TEXT("Laser");

	/** 현재 상호 작용 택스트 */
	FString InteractionDescription;

	/** 현재 상호 작용이 Hold 모드인지 여부 */
	uint8 bIsInteractionHoldMode : 1;

	/** 그로기 상태 상호 작용 택스트. 구조 상황에 출력 */
	UPROPERTY(EditDefaultsOnly, Category= "Character|Interaction", meta = (AllowPrivateAccess = "true"))
	FString GroggyInteractionDescription;

	/** 사망 상태 상호 작용 택스트. 시체 들기 상황에 출력 */
	UPROPERTY(EditDefaultsOnly, Category= "Character|Interaction", meta = (AllowPrivateAccess = "true"))
	FString DeathGrabDescription;

	/** 사망 상태에서 시체 놓기 상황에서 출력 */
	UPROPERTY(EditDefaultsOnly, Category= "Character|Interaction", meta = (AllowPrivateAccess = "true"))
	FString DeathGrabReleaseDescription;

	/** 사망 상태에서 시체를 들은 캐릭터. Bind Character가 존재하면 BoundCharacter이다. */
	UPROPERTY(ReplicatedUsing = OnRep_BindCharacter, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AUnderwaterCharacter> BindCharacter;

	/** 현재 들고 있는 시체 캐릭터. 한 번에 여러개의 시체를 들 수 있다. */
	UPROPERTY(ReplicatedUsing = OnRep_BoundCharacters, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<AUnderwaterCharacter>> BoundCharacters;

	/** 시체 로프를 연결할 CableBindingActor 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ACableBindingActor> CableBindingActorClass;

	/** 시체를 연결하는 로프 클래스 엑터, BoundCharacter가 소유하고 있다. */
	UPROPERTY()
	TObjectPtr<class ACableBindingActor> CableBindingActor;

	/** 로프에 바인드할 때마다 속도 감소 수치. 0.15일 경우 Bound Characters의 개수마다 15%씩 속도가 감소한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", ClampMax = "1.0", ClampMin = "0.0"))
	float BindMultiplier;

	uint8 bIsAttackedByEyeStalker : 1;

	/** Post Process를 관리하는 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UPostProcessSettingComponent> PostProcessSettingComponent;

	/** 캐릭터가 사망했을 때 관전으로 전이할 Timer */
	FTimerHandle DeathTimer;

	/** 캐릭터가 사망했을 떄 관전으로 전이되기까지 걸리는 시간 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float DeathTransitionTime = 1.0f;

	TWeakObjectPtr<class USoundSubsystem> SoundSubsystem;
	
	uint8 bIsMovementBlockedByTutorial : 1;

	/** 캐릭터 상태 잠금에 대한 bool. 변수 Tutorial에서 상태가 바뀌지 않도록 하는데 사용 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	uint8 bIsDeathLocked : 1;

	
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

	/** 캐릭터의 현재 상태를 반환. Normal, Groggy, Death... */
	FORCEINLINE UCombatEffectComponent* GetCombatEffectComponent() const { return CombatEffectComponent; }

	FORCEINLINE URadar2DComponent* GetRadarComponent() const { return RadarComponent; }

	/** 캐릭터의 현재 상태를 반환 */
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }

	/** 캐릭터 무적 상태를 반환. 현재는 Server에서만 유효하다. */
	FORCEINLINE bool IsInvincible() const { return bIsInvincible; }

	/** 캐릭터 무적 상태를 설정. 현재는 Server에서만 유효하다. */
	FORCEINLINE void SetInvincible(const bool bNewInvincible) { bIsInvincible = bNewInvincible; }

	/** 캐릭터가 일반 상태인지 여부를 반환 */
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsNormal() const { return CharacterState == ECharacterState::Normal; }

	/** 캐릭터가 Groggy 상태인지 여부를 반환 */
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsGroggy() const { return CharacterState == ECharacterState::Groggy; }

	/** 캐릭터가 Death 상태인지 여부를 반환 */
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsDeath() const { return CharacterState == ECharacterState::Death; }

	/** 캐릭터가 현재 살아있는지 여부를 반환. 살아 있으면 타겟팅될 수 있다. 사망이 아닌 상태를 의미한다. */
	UFUNCTION(BlueprintCallable)
	bool IsAlive() const;

	/** 캐릭터 상태를 설정한다. Server에서만 실행 가능하다. */
	void SetCharacterState(ECharacterState NewCharacterState);

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

	/** 현재 Bound된 Character를 반환. 복사본을 반환한다. */
	TArray<AUnderwaterCharacter*> GetBoundCharacters() const { return BoundCharacters; }

	/** Player Index를 반환 */
	FORCEINLINE int GetPlayerIndex() const { return PlayerIndex; }

	/** 현재 Eye Stalker에게 공격받았는지 여부를 설정 */
	FORCEINLINE void SetIsAttackedByEyeStalker(const bool bNewAttacked) { bIsAttackedByEyeStalker = bNewAttacked; }

	/** 현재 Eye Stalker에게 공격받았는지 여부를 반환 */
	FORCEINLINE bool IsAttackedByEyeStalker() const { return bIsAttackedByEyeStalker; }

	/** Post Process Setting Component를 반환 */
	FORCEINLINE UPostProcessSettingComponent* GetPostProcessSettingComponent() const { return PostProcessSettingComponent; }

	/** 현재 캐릭터를 어그로로 설정하고 있는 Targeting Actor의 개수를 반환 */
	FORCEINLINE int GetTargetingActorCount() const { return TargetingActors.Num(); }

	/** 캐릭터가 현재 Capture State 인지 여부를 반환 */
	FORCEINLINE bool IsCaptured() const { return bIsCaptured; }

	/** 캐릭터의 소유자 AADPlayerController 반환 */
	FORCEINLINE AADPlayerController* GetOwnerController() const { return OwnerController; }

	/** 깊이 컴포넌트를 반환 */
	FORCEINLINE UDepthComponent* GetDepthComponent() const { return DepthComponent; }

	FORCEINLINE UInputAction* GetSprintAction() const { return SprintAction; }
	FORCEINLINE UInputAction* GetRadarAction() const { return RadarAction; }
	FORCEINLINE UInputAction* GetInteractAction() const { return InteractionAction; }
	FORCEINLINE UInputAction* GetLightToggleAction() const { return LightAction; }
	FORCEINLINE UInputAction* GetReloadAction() const { return ReloadAction; } 
	FORCEINLINE UInputAction* GetSelectInventorySlot1() const { return EquipSlot1Action; }
	FORCEINLINE UInputAction* GetSelectInventorySlot2() const { return EquipSlot2Action; }
	FORCEINLINE UInputAction* GetSelectInventorySlot3() const { return EquipSlot3Action; }

	void SetMovementBlockedByTutorial(bool bIsBlocked);

	/** 캐릭터 상태 변환을 잠그는 함수 */
	FORCEINLINE void SetIsCharacterStateLocked(const bool bNewDeathLocked) { bIsDeathLocked = bNewDeathLocked; }
	
	/** 현재 캐릭터의 이동 방향을 반환. Server와 Client가 동기화되어 있다. */
	FORCEINLINE EMoveDirection GetMoveDirection() const { return MoveDirection; }

	/** 현재 캐릭터가 스프린트를 할 수 있는지 여부를 반환. 전방 이동이 있을 경우에만 스프린트를 할 수 있다. Server / Client 복제됨 */
	bool CanSprint() const;

	/** 캐릭터의 채광 상태를 변경하는 함수*/
	FORCEINLINE void SetIsMining(bool bNewIsMining) { bIsMining = bNewIsMining; }
	
	/** 캐릭터의 현재 채광 상태를 반환하는 함수*/
	FORCEINLINE bool IsMining() { return bIsMining; }
protected:

	class USoundSubsystem* GetSoundSubsystem();
	
#pragma endregion
};
