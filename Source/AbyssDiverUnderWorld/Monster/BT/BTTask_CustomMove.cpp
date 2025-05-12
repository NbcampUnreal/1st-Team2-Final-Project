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
	bNotifyTaskFinished = true; // Ensuring OnTaskFinished execution
}

EBTNodeResult::Type UBTTask_CustomMove::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	CashedOwnerComp = &OwnerComp;

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	APawn* AIPawn = AIController->GetPawn();
	if (!AIPawn) return EBTNodeResult::Failed;

	CashedPathComp = AIPawn->FindComponentByClass<UFlyingAIPathfindingBase>();
	if (!CashedPathComp) return EBTNodeResult::Failed;

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp) return EBTNodeResult::Failed;

	FVector TargetLocation = BlackboardComp->GetValueAsVector("TargetLocation");

	// Arrived Event Binding
	CashedPathComp->OnFinishedMoving.AddDynamic(this, &UBTTask_CustomMove::HandleMoveFinishied);
	if (AIPawn->HasAuthority())
	{
		CashedPathComp->S_MoveTo(TargetLocation);
	}
	else
	{
		LOG(TEXT("Tried to call Server_MoveTo on client"));
	}

	return EBTNodeResult::InProgress;
}
void UBTTask_CustomMove::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	if (CashedPathComp)
	{
		CashedPathComp->OnFinishedMoving.RemoveDynamic(this, &UBTTask_CustomMove::HandleMoveFinishied);
	}
}
void UBTTask_CustomMove::HandleMoveFinishied()
{
	if (CashedOwnerComp)
	{
		FinishLatentTask(*CashedOwnerComp, EBTNodeResult::Succeeded);
	}
}

