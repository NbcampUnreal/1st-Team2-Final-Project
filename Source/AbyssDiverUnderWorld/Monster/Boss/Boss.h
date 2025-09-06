#pragma once

#include "CoreMinimal.h"

#include "BossAIController.h"
#include "Monster/Monster.h"

#include "Boss.generated.h"

class AEnhancedBossAIController;
class UNiagaraSystem;
enum class EBossPhysicsType : uint8;

class ATargetPoint;
class AUnderwaterCharacter;
class UAquaticMovementComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API ABoss : public AMonster
{
	GENERATED_BODY()
	
public:
	ABoss();

protected:
	virtual void BeginPlay() override;
	//virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

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
	//FVector GetRandomNavMeshLocation(const FVector& Origin, const float& Radius) const;

	/** 새로운 목표 이동지점을 할당하는 함수 */
	//virtual void SetNewTargetLocation() override;

	/** 상하좌우 방향으로 라인 트레이싱을 한다.
	 *
	 * 장애물이 탐지될 경우 벽의 표면벡터에 맞도록 액터를 회전시킨다.
	 *
	 * @param InDeltaTime - 프레임 시간
	 */
	//void SmoothMoveAlongSurface(const float& InDeltaTime);

	/** TargetLocation 방향으로 회전하며 이동하는 함수
	 * 
	 * @param InDeltaTime - 프레임 시간
	 */
	//virtual void PerformNormalMovement(const float& InDeltaTime) override;

	/** TargetPlayer를 추적하며 이동하는 함수
	 * 
	 * TargetPlayer가 존재하는 경우, TargetPlayer의 위치를 추적하여 이동한다.
	 * 
	 * @param InDeltaTime - 프레임 시간
	 */
	//void PerformChasing(const float& InDeltaTime);

	/** bIsTurning 변수를 true로 설정하고 회전 시작
	 * 
	 * 회전 중인 경우 PerformTurn 함수를 호출하여 회전을 수행한다.
	 */
	//void StartTurn();

	/** 현재 액터가 NavMesh 상에 존재하지 않는 경우 NavMesh 상의 유효한 위치로 이동
	 * 
	 * NavMesh 상에 존재하지 않는 경우, NavMesh 상의 가장 가까운 위치로 이동한다.
	 */
	//void ReturnToNavMeshArea();

	/** 회전 중인 경우 회전 수행
	 * 
	 * 회전 중인 경우에만 호출되며, 회전이 끝나면 bIsTurning 변수를 false로 설정한다.
	 * @param InDeltaTime - 프레임 시간
	 */
	//void PerformTurn(const float& InDeltaTime);

	/** 현재 액터의 전방에 장애물이 있는지 확인하는 함수
	 * @return 전방에 장애물이 있는 경우 true, 그렇지 않은 경우 false 반환
	 */
	//bool HasObstacleAhead();

	/** 현재 액터가 NavMesh 상에 존재하는지 확인하는 함수
	 * @param InLocation - 검사할 위치
	 * @return NavMesh 상에 존재하는 경우 true, 그렇지 않은 경우 false 반환
	 */
	//bool IsLocationOnNavMesh(const FVector& InLocation) const;

	/** 캐릭터의 이동 설정을 변경하는 함수
	 * @param InBrakingDecelerationSwimming: 수영 중 감속값
	 * @param InMaxSwimSpeed: 최대 수영 속도
	 */
	//void SetCharacterMovementSetting(const float& InBrakingDecelerationSwimming, const float& InMaxSwimSpeed);

	/** 캐릭터의 이동 설정을 초기화하는 함수
	 * 
	 * 기본값으로 설정된 수영 중 감속값과 최대 수영 속도로 초기화한다.
	 */
	/*void InitCharacterMovementSetting();*/

	/** NavMesh 기반으로 랜덤 위치를 찾는 함수
	 * @return NavMesh 상의 랜덤 위치 반환
	 */
	FVector GetNextPatrolPoint();

	/** 보스의 상태를 초기화하는 함수
	 * @param State: 초기화할 상태
	 */
	//void SetBossState(EBossState State);

	/** 플레이어를 밀치는 함수
	 * @param Player: 밀칠 플레이어
	 * @param Power: 밀치는 힘
	 */
	//void LaunchPlayer(AUnderwaterCharacter* Player, const float& Power) const;
	
	/** 데미지를 받을 때 호출하는 함수 */
	
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(NetMulticast, Unreliable)
	void M_PlayBloodEffect(const FVector& Location, const FRotator& Rotation);
	void M_PlayBloodEffect_Implementation(const FVector& Location, const FRotator& Rotation);
	
	/** 보스의 체력이 0이하로 떨어지는 경우 사망 상태로 전이 */
	virtual void OnDeath() override;

	/** 보스를 타겟 방향으로 회전시키는 함수 */
	virtual void RotationToTarget(AActor* Target);
	virtual void RotationToTarget(const FVector& InTargetLocation);

	/** 보스의 공격 시 애니메이션 재생*/
	//virtual void Attack() override;
	virtual void OnAttackEnded() override;

	/** 보스의 공격이 끝난 후 타격 판정을 초기화하는 함수
	 *
	 * AnimNotify_BossAttack 호출 후 일정 시간 후 호출
	 */

	/** 보스의 이동속도를 설정하는 함수 */
	void SetMoveSpeed(const float& SpeedMultiplier);

	
	//UFUNCTION(NetMulticast, Reliable)
	//virtual void M_PlayAnimation(UAnimMontage* AnimMontage, float InPlayRate = 1, FName StartSectionName = NAME_None);
	////virtual void M_PlayAnimation_Implementation(UAnimMontage* AnimMontage, float InPlayRate = 1, FName StartSectionName = NAME_None);

	//UFUNCTION(NetMulticast, Reliable)
	//void M_OnDeath();
	virtual void M_OnDeath_Implementation() override;


private:
	
#pragma endregion

#pragma region Variable
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Stat")
	EBossPhysicsType BossPhysicsType;

	//UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Effect")
	//TObjectPtr<UNiagaraSystem> BloodEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Stat")
	float MinPatrolDistance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Stat")
	float MaxPatrolDistance;

	//UPROPERTY(VisibleAnywhere)
	//TObjectPtr<UAnimInstance> AnimInstance;

	//UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Animation")
	//TArray<TObjectPtr<UAnimMontage>> AttackAnimations;

	//UPROPERTY()
	//float ChaseAccumulatedTime = 0.0f;
	
	/** 새로운 물리 기반 수중 이동 컴포넌트 */
	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Boss|Movement")
	//TObjectPtr<UAquaticMovementComponent> AquaticMovementComponent;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Stat")
	float TraceDistance = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Stat")
	float TurnTraceDistance = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Stat")
	float FourDirectionTraceDistance = 300.0f;

	//UPROPERTY(Replicated, BlueprintReadWrite)
	//EMonsterState BossState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	uint8 bEnableDownTrace : 1 = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	uint8 bEnableHorizontalTrace : 1 = true;

	UPROPERTY()
	FVector DamagedLocation;

	UPROPERTY()
	TObjectPtr<AEnhancedBossAIController> EnhancedAIController;

	FCollisionQueryParams Params;

private:
	//static const FName BossStateKey;
	
	//uint8 bIsAttackCollisionOverlappedPlayer : 1;
	float TurnTimer = 0.0f;
	/*float OriginDeceleration;*/
	float SphereOverlapRadius = 100.0f;
	FVector CachedSpawnLocation;
	FVector TurnDirection;

#pragma endregion

#pragma region Getter, Setter
public:
	/** Target Getter, Setter */
	//FORCEINLINE bool GetIsAttackCollisionOverlappedPlayer() const { return bIsAttackCollisionOverlappedPlayer; };

	FORCEINLINE UAnimInstance* GetAnimInstance() const { return AnimInstance; }
	FORCEINLINE FVector GetDamagedLocation() const { return DamagedLocation; }
	FORCEINLINE FVector GetCachedSpawnLocation() const { return CachedSpawnLocation; }

#pragma endregion
	
};