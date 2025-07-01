// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_RotateToLocation.generated.h"


UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_RotateToLocation : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	

public:
	UBTTask_RotateToLocation();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;


#pragma region Variable
protected:
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
	UPROPERTY()
	FVector TargetLocation;
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector TargetLocationKey;
#pragma endregion
};
