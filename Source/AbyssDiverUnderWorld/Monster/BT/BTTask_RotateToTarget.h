// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_RotateToTarget.generated.h"


UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_RotateToTarget : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	

public:
	UBTTask_RotateToTarget();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;


#pragma region Variable
protected:
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
	UPROPERTY()
	TObjectPtr<AActor> CachedTargetActor;
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector TargetActorKey;
#pragma endregion
};

