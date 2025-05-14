#pragma once

#include "CoreMinimal.h"
#include "BossAIController.h"
#include "Character/UnitBase.h"
#include "Boss.generated.h"

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
	void SetBossState(EBossState State);
	void LaunchPlayer(AUnderwaterCharacter* Player);
	
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
	
	/** 다음 순찰 지점으로 변환 */ 
	void AddPatrolPoint();

	/** 보스의 이동속도를 설정하는 함수 */
	UFUNCTION(BlueprintImplementableEvent)
	void SetMoveSpeed(float Speed);
	
	UFUNCTION(NetMulticast, Reliable)
	virtual void M_PlayAnimation(UAnimMontage* AnimMontage, float InPlayRate = 1, FName StartSectionName = NAME_None);
	virtual void M_PlayAnimation_Implementation(UAnimMontage* AnimMontage, float InPlayRate = 1, FName StartSectionName = NAME_None);

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
private:
	
#pragma endregion

#pragma region Variable
public:
	UPROPERTY()
	uint8 bIsAttackCollisionOverlappedPlayer : 1;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAnimInstance> AnimInstance;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Boss|Target")
	TObjectPtr<AUnderwaterCharacter> TargetPlayer;

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
	
protected:
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

private:
	static const FName BossStateKey;
	uint8 CurrentPatrolPointIndex = 0;
	uint8 bIsBiteAttackSuccess : 1;
	
#pragma endregion

#pragma region Getter, Setter
public:
	AUnderwaterCharacter* GetTarget();
	void SetTarget(AUnderwaterCharacter* Target);
	void InitTarget();
	
	void SetLastDetectedLocation(const FVector& InLastDetectedLocation);

	AActor* GetTargetPoint();
	FVector GetTargetPointLocation();

	bool GetIsAttackCollisionOverlappedPlayer();

	UCameraControllerComponent* GetCameraControllerComponent() const;

	FORCEINLINE bool GetIsBiteAttackSuccess() const { return bIsBiteAttackSuccess; }
	FORCEINLINE void SetIsBiteAttackFalse() { bIsBiteAttackSuccess = false; }

#pragma endregion
	
};