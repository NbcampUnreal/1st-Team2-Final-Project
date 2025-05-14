// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BT/BTTask_FindNextPatrolLocation.h"
#include "AIController.h"
#include "Monster/FlyingAIPathfindingBase.h"
#include "Monster/Monster.h"
#include "AbyssDiverUnderWorld.h"

UBTTask_FindNextPatrolLocation::UBTTask_FindNextPatrolLocation()
{
	NodeName = "Find Next Patrol Location";
	bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_FindNextPatrolLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	APawn* AIPawn = AIController->GetPawn();
	if (!AIPawn) return EBTNodeResult::Failed;

	AMonster* Monster = Cast<AMonster>(AIPawn);
	if (!Monster) return EBTNodeResult::Failed;

	UFlyingAIPathfindingBase* Pathfinding = AIPawn->FindComponentByClass<UFlyingAIPathfindingBase>();
	if (!Pathfinding) return EBTNodeResult::Failed;

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp) return EBTNodeResult::Failed;

	int32 Index = BlackboardComp->GetValueAsInt(PatrolIndexKey.SelectedKeyName);
	FVector TargetLocation = Monster->GetPatrolLocation(Index);

	BlackboardComp->SetValueAsVector(TargetLocationKey.SelectedKeyName, TargetLocation);
	BlackboardComp->SetValueAsInt(PatrolIndexKey.SelectedKeyName, Monster->GetNextPatrolIndex(Index));
	LOG(TEXT("TargetLocation : %s"), *TargetLocation.ToString())

	return EBTNodeResult::Succeeded;
}