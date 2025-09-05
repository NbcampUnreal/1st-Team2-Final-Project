// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"

#include "BTTask_ChaseByState.generated.h"

// NodeMemory는 bCreateNodeInstance를 false로 하더라도 몬스터마다 독립적
struct FBTChaseByStateTaskMemory
{
	TWeakObjectPtr<class AMonster> Monster;
	TWeakObjectPtr<class AMonsterAIController> AIController;
	float AccumulatedTime = 0.0f;
	float ChasingTime = 0.0f;
	uint8 bHasAttacked : 1 = false;
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_ChaseByState : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_ChaseByState();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTChaseByStateTaskMemory); }

private:
	// bCreateNodeInstance를 false로 설정했으므로 Task클래스 객체는 모든 몬스터가 공유한다
	// 따라서 A몬스터가 해당 값을 수정하면 B몬스터에도 영향감.
	UPROPERTY(EditAnywhere)
	uint8 MaxChasingTime = 20;
	
	UPROPERTY(EditAnywhere)
	uint8 MinChasingTime = 10;
};
