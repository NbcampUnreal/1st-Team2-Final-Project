// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_ChasePlayer.generated.h"

struct FBTChasePlayerTaskMemory
{
	TWeakObjectPtr<class AMonsterAIController> AIController;
	TWeakObjectPtr<class AMonster> Monster;
	float AccumulatedTime = 0.0f;
	float ChasingTime = 0.0f;
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_ChasePlayer : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTTask_ChasePlayer();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTChasePlayerTaskMemory); }

#pragma region Variable
private:
	UPROPERTY(EditAnywhere)
	uint8 MaxChasingTime = 20;

	UPROPERTY(EditAnywhere)
	uint8 MinChasingTime = 10;

	static const FName bCanAttackKey;
	static const FName bIsHidingKey;
	static const FName bIsPlayerHiddenKey;
	static const FName TargetPlayerKey;

	uint8 bHasAttacked : 1 = false;

#pragma endregion
};
