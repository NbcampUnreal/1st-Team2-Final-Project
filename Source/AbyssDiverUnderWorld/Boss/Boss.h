#pragma once

#include "CoreMinimal.h"
#include "BossAIController.h"
#include "Character/UnitBase.h"
#include "Boss.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API ABoss : public AUnitBase
{
	GENERATED_BODY()
	
public:
	ABoss();

protected:
	virtual void BeginPlay() override;

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

	/** 보스가 타겟을 향해 공격하는 함수 */
	virtual void Attack();

	/** 보스의 이동속도를 설정하는 함수 */
	UFUNCTION(BlueprintImplementableEvent)
	void SetMoveSpeed(float Speed);
	
	UFUNCTION(NetMulticast, Reliable)
	void M_PlayAnimation(UAnimMontage* AnimMontage, float InPlayRate = 1, FName StartSectionName = NAME_None);
	void M_PlayAnimation_Implementation(UAnimMontage* AnimMontage, float InPlayRate = 1, FName StartSectionName = NAME_None);
	
protected:
private:
	
#pragma endregion

#pragma region Variable
public:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAnimInstance> AnimInstance;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Boss|Target")
	TObjectPtr<APawn> TargetPlayer;

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

private:
	static const FName BossStateKey;
	
#pragma endregion

#pragma region Getter, Setter
public:
	APawn* GetTarget();
	void SetTarget(APawn* Target);
	
	void SetLastDetectedLocation(const FVector& InLastDetectedLocation);

#pragma endregion
	
};
