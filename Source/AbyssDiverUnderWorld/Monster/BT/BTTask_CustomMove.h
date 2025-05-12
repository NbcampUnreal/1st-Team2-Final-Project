// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_CustomMove.generated.h"

class UFlyingAIPathfindingBase;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_CustomMove : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTTask_CustomMove();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

#pragma region Method
protected:
	UFUNCTION()
	void HandleMoveFinishied(); // ArrivedEvent

#pragma endregion

#pragma region Variable
private:
	UPROPERTY()
	UFlyingAIPathfindingBase* CashedPathComp; // For disabling on arrival. raw point : because depended by pawn. Don'y worry about GC
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> CashedOwnerComp; // For FinishLatentTask

#pragma endregion
};
