// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_DefaultAttack.generated.h"


UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_DefaultAttack : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTTask_DefaultAttack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

#pragma region Method
protected:
	UFUNCTION()
	void HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

#pragma endregion

#pragma region Variable
private:
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector MonsterStateKey;

	uint8 bIsAttacking : 1;

#pragma endregion
};
