// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_MoveToPredictedTarget.generated.h"


UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_MoveToPredictedTarget : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTTask_MoveToPredictedTarget();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float PredictTime;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float AcceptanceRadius;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float MoveUpdateThreshold;
};
