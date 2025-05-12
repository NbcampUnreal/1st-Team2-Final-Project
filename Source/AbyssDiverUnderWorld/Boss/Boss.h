#pragma once

#include "CoreMinimal.h"
#include "BossAIController.h"
#include "Character/UnitBase.h"
#include "Boss.generated.h"

class AUnderwaterCharacter;

UCLASS()
class ABYSSDIVERUNDERWORLD_API ABoss : public AUnitBase
{
	GENERATED_BODY()
	
public:
	ABoss();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

#pragma region Method
public:
	/** 데미지를 받을 때 호출하는 함수 */
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	
	/** 보스의 체력이 0이하로 떨어지는 경우 사망 상태로 전이 */
	virtual void OnDeath();
	
	/** 보스를 타겟 방향으로 회전시키는 함수 */
	virtual void RotationToTarget();
	
	/** 보스를 움직이게 하고 Move Animation 재생 */
	virtual void Move();

	/** 보스의 움직임을 정지하고 Idle Animation 재생 */
	virtual void MoveStop();

	/** 보스가 감지한 타겟을 추적하는 함수 */
	virtual void MoveToTarget();

	/** 보스가 마지막으로 감지한 타겟의 위치를 추적하는 함수 */
	virtual void MoveToLastDetectedLocation();

	/** 보스의 공격 시 애니메이션 재생*/
	virtual void Attack();

	/** 보스의 공격이 끝난 후 타격 판정을 초기화하는 함수
	 *
	 * AnimNotify_BossAttack 호출 후 일정 시간 후 호출
	 */
	virtual void OnAttackEnded();

	/** 보스의 이동속도를 설정하는 함수 */
	UFUNCTION(BlueprintImplementableEvent)
	void SetMoveSpeed(float Speed);
	
	UFUNCTION(NetMulticast, Reliable)
	void M_PlayAnimation(UAnimMontage* AnimMontage, float InPlayRate = 1, FName StartSectionName = NAME_None);
	void M_PlayAnimation_Implementation(UAnimMontage* AnimMontage, float InPlayRate = 1, FName StartSectionName = NAME_None);

	UFUNCTION()
	void OnMeshOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
							UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
							bool bFromSweep, const FHitResult& SweepResult);
	
protected:
private:
	
#pragma endregion

#pragma region Variable
public:
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
	
protected:
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

private:
	static const FName BossStateKey;
	
#pragma endregion

#pragma region Getter, Setter
public:
	APawn* GetTarget();
	void SetTarget(AUnderwaterCharacter* Target);
	
	void SetLastDetectedLocation(const FVector& InLastDetectedLocation);

#pragma endregion
	
};
