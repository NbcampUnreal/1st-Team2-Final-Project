// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BT/BTTask_DefaultAttack.h"
#include "AIController.h"

UBTTask_DefaultAttack::UBTTask_DefaultAttack()
{
	NodeName = "DefaultAttack";
	bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_DefaultAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	APawn* AIPawn = AIController->GetPawn();
	if (!AIPawn) return EBTNodeResult::Failed;



	return EBTNodeResult::Type();
}
