// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_HorrorCreatureAttack.generated.h"


UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_HorrorCreatureAttack : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTTask_HorrorCreatureAttack();

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
	UPROPERTY()
	TObjectPtr<class AHorrorCreature> CachedHorrorCreature;

protected:
	UPROPERTY(EditAnywhere)
	struct FBlackboardKeySelector MonsterStateKey;

#pragma endregion
};
