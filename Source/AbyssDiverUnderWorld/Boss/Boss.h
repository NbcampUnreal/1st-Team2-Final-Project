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
	/** 보스를 움직이게 하고 Move Animation 재생 */
	virtual void Move();

	/** 보스의 움직임을 정지하고 Idle Animation 재생 */
	virtual void MoveStop();
	
protected:
	virtual void SetMoveTimer();
	virtual void SetMoveStopTimer();

	UFUNCTION(NetMulticast, Reliable)
	void M_PlayAnimation(UAnimMontage* AnimMontage, float InPlayRate = 1, FName StartSectionName = NAME_None);
	void M_PlayAnimation_Implementation(UAnimMontage* AnimMontage, float InPlayRate = 1, FName StartSectionName = NAME_None);
private:
	
#pragma endregion

#pragma region Variable
public:

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Blackboard")
	TObjectPtr<UBlackboardComponent> BlackboardComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|AI")
	TObjectPtr<ABossAIController> AIController;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Animation")
	TObjectPtr<UAnimMontage> IdleAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Animation")
	TObjectPtr<UAnimMontage> MoveAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Animation")
	TArray<TObjectPtr<UAnimMontage>> NormalAttackAnimations;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Animation")
	TArray<TObjectPtr<UAnimMontage>> SpecialAttackAnimations;

private:
	FTimerHandle MovementTimer;
	
#pragma endregion

#pragma region Getter, Setter
public:

#pragma endregion
	
};
