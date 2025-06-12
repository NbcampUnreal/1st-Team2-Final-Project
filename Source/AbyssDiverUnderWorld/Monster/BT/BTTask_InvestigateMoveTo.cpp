// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BT/BTTask_InvestigateMoveTo.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Monster/EMonsterState.h"
#include "Monster/Monster.h"

UBTTask_InvestigateMoveTo::UBTTask_InvestigateMoveTo()
{
	NodeName = "Investigate Move To";
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	AcceptableRadius = 200.0f;
}

EBTNodeResult::Type UBTTask_InvestigateMoveTo::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return EBTNodeResult::InProgress;
}

void UBTTask_InvestigateMoveTo::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* AIPawn = AIController ? AIController->GetPawn() : nullptr;
	AMonster* Monster = Cast<AMonster>(AIPawn);

	if (!Blackboard || !AIController || !AIPawn || !Monster)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (Monster->TargetActor)
	{
		Monster->SetMonsterState(EMonsterState::Chase);
		Blackboard->ClearValue(InvestigateLocationKey.SelectedKeyName);
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	const FVector InvestigateLocation = Blackboard->GetValueAsVector(InvestigateLocationKey.SelectedKeyName);
	const FVector CurrentLocation = AIPawn->GetActorLocation();
	const float Distance = FVector::Dist(InvestigateLocation, CurrentLocation);

	if (Distance <= AcceptableRadius)
	{
		Monster->SetMonsterState(EMonsterState::Patrol);
		Blackboard->ClearValue(InvestigateLocationKey.SelectedKeyName);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
