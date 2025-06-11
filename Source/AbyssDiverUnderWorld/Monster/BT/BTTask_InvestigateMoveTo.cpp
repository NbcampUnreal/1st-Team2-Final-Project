// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BT/BTTask_InvestigateMoveTo.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Monster/EMonsterState.h"
#include "Monster/Monster.h"

UBTTask_InvestigateMoveTo::UBTTask_InvestigateMoveTo()
{
	NodeName = "Investigate Move To";
	bNotifyTick = false;
	AcceptableRadius = 200.0f;
}

EBTNodeResult::Type UBTTask_InvestigateMoveTo::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard) return EBTNodeResult::Failed;
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;
	APawn* AIPawn = AIController->GetPawn();
	if (!AIPawn) return EBTNodeResult::Failed;

	const FVector InvestigateLocation = Blackboard->GetValueAsVector(InvestigateLocationKey.SelectedKeyName);
	const FVector MonsterLocation = AIPawn->GetActorLocation();

	const float Distance = FVector::Dist(InvestigateLocation, MonsterLocation);

	if (Distance <= AcceptableRadius)
	{
		AMonster* Monster = Cast<AMonster>(AIPawn);
		Blackboard->SetValueAsEnum(MonsterStateKey.SelectedKeyName, static_cast<uint8>(EMonsterState::Patrol));

		if (Monster)
		{
			Monster->SetMonsterState(EMonsterState::Patrol); 
		}
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
