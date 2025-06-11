// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_InvestigateMoveTo.generated.h"


UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_InvestigateMoveTo : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTTask_InvestigateMoveTo();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector MonsterStateKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector InvestigateLocationKey;

	UPROPERTY(EditAnywhere)
	float AcceptableRadius;
};
