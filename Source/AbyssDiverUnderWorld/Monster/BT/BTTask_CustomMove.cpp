// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BT/BTTask_CustomMove.h"
#include "AIController.h"
#include "Monster/FlyingAIPathfindingBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbyssDiverUnderWorld.h"

UBTTask_CustomMove::UBTTask_CustomMove()
{
	NodeName = "Custom Move To Location";
	bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_CustomMove::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	APawn* AIPawn = AIController->GetPawn();
	if (!AIPawn) return EBTNodeResult::Failed;

	UFlyingAIPathfindingBase* PathfindingComp = AIPawn->FindComponentByClass<UFlyingAIPathfindingBase>();
	if (!PathfindingComp) return EBTNodeResult::Failed;

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp) return EBTNodeResult::Failed;

	FVector TargetLocation = BlackboardComp->GetValueAsVector("TargetLocation");

	if (AIPawn->HasAuthority())
	{
		PathfindingComp->S_MoveTo(TargetLocation);
	}
	else
	{
		LOG(TEXT("Tried to call Server_MoveTo on client"));
	}

	return EBTNodeResult::Succeeded;
}	
