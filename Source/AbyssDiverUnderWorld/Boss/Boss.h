#pragma once

#include "CoreMinimal.h"
#include "AbyssDiverUnderWorld.h"
#include "BossAIController.h"
#include "Character/UnitBase.h"
#include "Boss.generated.h"

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
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

#pragma region Method
public:
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
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	
	/** 보스의 체력이 0이하로 떨어지는 경우 사망 상태로 전이 */
	virtual void OnDeath();

	/** 보스를 타겟 방향으로 회전시키는 함수 */
	virtual void RotationToTarget(AActor* Target);
	virtual void RotationToTarget(const FVector& TargetLocation);

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

protected:
	void ApplyPhysicsSimulation();
	
private:
	
#pragma endregion

#pragma region Variable
public:
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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Boss|Target")
	FVector LastDetectedLocation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Animation")
	TObjectPtr<UAnimMontage> DetectedAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Animation")
	TObjectPtr<UAnimMontage> DeathAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Animation")
	TArray<TObjectPtr<UAnimMontage>> NormalAttackAnimations;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Animation")
	TArray<TObjectPtr<UAnimMontage>> SpecialAttackAnimations;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Collision")
	TObjectPtr<UCapsuleComponent> AttackCollision;

	UPROPERTY()
	float ChaseAccumulatedTime = 0.0f;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Stat")
	uint8 bIsAttackInfinite : 1;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Boss|Camera")
	TObjectPtr<UCameraControllerComponent> CameraControllerComponent;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Boss|PatrolPoints")
	TArray<TObjectPtr<ATargetPoint>> PatrolPoints;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Blackboard")
	TObjectPtr<UBlackboardComponent> BlackboardComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|AI")
	TObjectPtr<ABossAIController> AIController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Stat")
	float AttackRadius;

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

	UPROPERTY()
	FVector DamagedLocation;

private:
	static const FName BossStateKey;
	uint8 CurrentPatrolPointIndex = 0;
	uint8 bIsBiteAttackSuccess : 1;
	uint8 bIsAttackCollisionOverlappedPlayer : 1;
	FVector CachedSpawnLocation;
	float OriginDeceleration;
	
#pragma endregion

#pragma region Getter, Setter
public:
	/** Target Getter, Setter */
	FORCEINLINE AUnderwaterCharacter* GetTarget() const { return TargetPlayer; };
	FORCEINLINE void SetTarget(AUnderwaterCharacter* Target) { TargetPlayer = Target; };
	FORCEINLINE void InitTarget() { TargetPlayer = nullptr; };

	/** LastDetectedLocation Getter */
	FORCEINLINE void SetLastDetectedLocation(const FVector& InLastDetectedLocation) { LastDetectedLocation = InLastDetectedLocation; };

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

	AActor* GetTargetPoint();
	const FVector GetTargetPointLocation() const;
	
#pragma endregion
	
};