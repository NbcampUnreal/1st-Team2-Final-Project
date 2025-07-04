#pragma once

#include "CoreMinimal.h"
#include "AbyssDiverUnderWorld.h"
#include "BossAIController.h"
#include "Character/UnitBase.h"
#include "Boss.generated.h"

class AEnhancedBossAIController;
class UNiagaraSystem;
enum class EBossPhysicsType : uint8;
class UCameraControllerComponent;
class ATargetPoint;
class AUnderwaterCharacter;

UCLASS()
class ABYSSDIVERUNDERWORLD_API ABoss : public AUnitBase
{
	GENERATED_BODY()
	
public:
	ABoss();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

#pragma region Method
public:
	/** NavMesh 상의 랜덤 위치를 반환하는 함수
	 * 
	 * NavMesh 상의 유효한 위치를 찾기 위해 NavMesh 시스템을 사용한다.
	 * 
	 * @param Origin - 시작 위치
	 * @param Radius - 탐색 반경
	 * @return NavMesh 상의 랜덤 위치 반환
	 */
	FVector GetRandomNavMeshLocation(const FVector& Origin, const float& Radius) const;

	/** 새로운 목표 이동지점을 할당하는 함수 */
	void SetNewTargetLocation();

	/** 상하좌우 방향으로 라인 트레이싱을 한다.
	 *
	 * 장애물이 탐지될 경우 벽의 표면벡터에 맞도록 액터를 회전시킨다.
	 *
	 * @param InDeltaTime - 프레임 시간
	 */
	void SmoothMoveAlongSurface(const float& InDeltaTime);

	/** TargetLocation 방향으로 회전하며 이동하는 함수
	 * 
	 * @param InDeltaTime - 프레임 시간
	 */
	void PerformNormalMovement(const float& InDeltaTime);

	/** TargetPlayer를 추적하며 이동하는 함수
	 * 
	 * TargetPlayer가 존재하는 경우, TargetPlayer의 위치를 추적하여 이동한다.
	 * 
	 * @param InDeltaTime - 프레임 시간
	 */
	void PerformChasing(const float& InDeltaTime);

	/** bIsTurning 변수를 true로 설정하고 회전 시작
	 * 
	 * 회전 중인 경우 PerformTurn 함수를 호출하여 회전을 수행한다.
	 */
	void StartTurn();

	/** 현재 액터가 NavMesh 상에 존재하지 않는 경우 NavMesh 상의 유효한 위치로 이동
	 * 
	 * NavMesh 상에 존재하지 않는 경우, NavMesh 상의 가장 가까운 위치로 이동한다.
	 */
	void ReturnToNavMeshArea();

	/** 회전 중인 경우 회전 수행
	 * 
	 * 회전 중인 경우에만 호출되며, 회전이 끝나면 bIsTurning 변수를 false로 설정한다.
	 * @param InDeltaTime - 프레임 시간
	 */
	void PerformTurn(const float& InDeltaTime);

	/** 현재 액터의 전방에 장애물이 있는지 확인하는 함수
	 * @return 전방에 장애물이 있는 경우 true, 그렇지 않은 경우 false 반환
	 */
	bool HasObstacleAhead();

	/** 현재 액터가 NavMesh 상에 존재하는지 확인하는 함수
	 * @param InLocation - 검사할 위치
	 * @return NavMesh 상에 존재하는 경우 true, 그렇지 않은 경우 false 반환
	 */
	bool IsLocationOnNavMesh(const FVector& InLocation) const;

	/** 캐릭터의 이동 설정을 변경하는 함수
	 * @param InBrakingDecelerationSwimming: 수영 중 감속값
	 * @param InMaxSwimSpeed: 최대 수영 속도
	 */
	void SetCharacterMovementSetting(const float& InBrakingDecelerationSwimming, const float& InMaxSwimSpeed);

	/** 캐릭터의 이동 설정을 초기화하는 함수
	 * 
	 * 기본값으로 설정된 수영 중 감속값과 최대 수영 속도로 초기화한다.
	 */
	void InitCharacterMovementSetting();

	/** NavMesh 기반으로 랜덤 위치를 찾는 함수
	 * @return NavMesh 상의 랜덤 위치 반환
	 */
	FVector GetNextPatrolPoint();

	/** 보스의 상태를 초기화하는 함수
	 * @param State: 초기화할 상태
	 */
	void SetBossState(EBossState State);

	/** 플레이어를 밀치는 함수
	 * @param Player: 밀칠 플레이어
	 * @param Power: 밀치는 힘
	 */
	void LaunchPlayer(AUnderwaterCharacter* Player, const float& Power) const;
	
	/** 데미지를 받을 때 호출하는 함수 */
	UFUNCTION(BlueprintCallable)
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(NetMulticast, Unreliable)
	void M_PlayBloodEffect(const FVector& Location, const FRotator& Rotation);
	void M_PlayBloodEffect_Implementation(const FVector& Location, const FRotator& Rotation);
	
	/** 보스의 체력이 0이하로 떨어지는 경우 사망 상태로 전이 */
	virtual void OnDeath();

	/** 보스를 타겟 방향으로 회전시키는 함수 */
	virtual void RotationToTarget(AActor* Target);
	virtual void RotationToTarget(const FVector& InTargetLocation);

	/** 보스의 공격 시 애니메이션 재생*/
	virtual void Attack();

	/** 보스의 공격이 끝난 후 타격 판정을 초기화하는 함수
	 *
	 * AnimNotify_BossAttack 호출 후 일정 시간 후 호출
	 */
	virtual void OnAttackEnded();

	/** 보스의 이동속도를 설정하는 함수 */
	void SetMoveSpeed(const float& SpeedMultiplier);
	
	UFUNCTION(NetMulticast, Reliable)
	virtual void M_PlayAnimation(UAnimMontage* AnimMontage, float InPlayRate = 1, FName StartSectionName = NAME_None);
	virtual void M_PlayAnimation_Implementation(UAnimMontage* AnimMontage, float InPlayRate = 1, FName StartSectionName = NAME_None);

	UFUNCTION(NetMulticast, Reliable)
	void M_OnDeath();
	void M_OnDeath_Implementation();

	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnMeshOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
							UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
							bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnBiteCollisionOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
							UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
							bool bFromSweep, const FHitResult& SweepResult);
							
	UFUNCTION()
	void OnAttackCollisionOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
							UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
							bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnAttackCollisionOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
							UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 몬스터 죽었을 때 레이더에서 Off 되도록
	UFUNCTION()
	void DeathToRaderOff();
protected:
	void ApplyPhysicsSimulation();
	
private:
	
#pragma endregion

#pragma region Variable
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Stat")
	float ChasingRotationSpeedMultiplier = 1.5f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Stat")
	float ChasingMovementSpeedMultiplier = 2.2f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Stat")
	float MovementInterpSpeed = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Stat")
	float RotationInterpSpeed;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Stat")
	EBossPhysicsType BossPhysicsType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Effect")
	TObjectPtr<UNiagaraSystem> BloodEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Stat")
	float MinPatrolDistance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Stat")
	float MaxPatrolDistance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Stat")
	float LaunchPower;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Stat")
	float AttackedCameraShakeScale;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAnimInstance> AnimInstance;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Boss|Target")
	TObjectPtr<AUnderwaterCharacter> TargetPlayer;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Boss|Target")
	TObjectPtr<AUnderwaterCharacter> CachedTargetPlayer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Animation")
	TArray<TObjectPtr<UAnimMontage>> AttackAnimations;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Collision")
	TObjectPtr<UCapsuleComponent> AttackCollision;

	UPROPERTY()
	float ChaseAccumulatedTime = 0.0f;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Stat")
	float TraceDistance = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Stat")
	float TurnTraceDistance = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Stat")
	float FourDirectionTraceDistance = 300.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MinTargetDistance = 100.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WanderRadius = 1300.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Stat")
	uint8 bIsAttackInfinite : 1;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Boss|Camera")
	TObjectPtr<UCameraControllerComponent> CameraControllerComponent;

	/** 공격 받은 플레이어 리스트
	 * 
	 * 중복 공격을 방지하기 위한 용도
	 * 
	 * 공격이 끝난 후 리스트는 초기화됨
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Boss|Attack")
	TArray<TObjectPtr<AUnderwaterCharacter>> AttackedPlayers;

	UPROPERTY(Replicated, BlueprintReadWrite)
	EBossState BossState;

	UPROPERTY(BlueprintReadWrite)
	float CurrentMoveSpeed = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	uint8 bDrawDebugLine : 1 = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	uint8 bEnableDownTrace : 1 = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	uint8 bEnableHorizontalTrace : 1 = true;

	UPROPERTY()
	FVector DamagedLocation;

	UPROPERTY()
	TObjectPtr<AEnhancedBossAIController> EnhancedAIController;

	FCollisionQueryParams Params;
	uint8 bIsTurning : 1 = false;
	uint8 bIsAttacking : 1 = false;

private:
	static const FName BossStateKey;
	uint8 bIsBiteAttackSuccess : 1;
	uint8 bIsAttackCollisionOverlappedPlayer : 1;
	float TurnTimer = 0.0f;
	float OriginDeceleration;
	float SphereOverlapRadius = 100.0f;
	FVector TargetLocation;
	FVector CachedSpawnLocation;
	FVector TurnDirection;

#pragma endregion

#pragma region Getter, Setter
public:
	/** Target Getter, Setter */
	FORCEINLINE AUnderwaterCharacter* GetTarget() const { return TargetPlayer; };
	FORCEINLINE void SetTarget(AUnderwaterCharacter* Target) { TargetPlayer = Target; };
	FORCEINLINE void InitTarget() { TargetPlayer = nullptr; };
	
	FORCEINLINE bool GetIsAttackCollisionOverlappedPlayer() const { return bIsAttackCollisionOverlappedPlayer; };

	FORCEINLINE UCameraControllerComponent* GetCameraControllerComponent() const { return CameraControllerComponent; };

	FORCEINLINE bool GetIsBiteAttackSuccess() const { return bIsBiteAttackSuccess; }
	FORCEINLINE void SetIsBiteAttackFalse() { bIsBiteAttackSuccess = false; }
	FORCEINLINE UAnimInstance* GetAnimInstance() const { return AnimInstance; }
	FORCEINLINE FVector GetDamagedLocation() const { return DamagedLocation; }
	FORCEINLINE AUnderwaterCharacter* GetCachedTarget() const { return CachedTargetPlayer; };
	FORCEINLINE void SetCachedTarget(AUnderwaterCharacter* Target) { CachedTargetPlayer = Target; };
	FORCEINLINE void InitCachedTarget() { CachedTargetPlayer = nullptr; };
	FORCEINLINE FVector GetCachedSpawnLocation() const { return CachedSpawnLocation; }

	FORCEINLINE void SetTargetLocation(const FVector& InTargetLocation) { TargetLocation = InTargetLocation; }

#pragma endregion
	
};