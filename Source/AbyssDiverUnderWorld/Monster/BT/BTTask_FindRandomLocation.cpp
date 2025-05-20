// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BT/BTTask_FindRandomLocation.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbyssDiverUnderWorld.h"

UBTTask_FindRandomLocation::UBTTask_FindRandomLocation()
{
	NodeName = "Find Random Location";
}

EBTNodeResult::Type UBTTask_FindRandomLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;
	
	APawn* AIPawn = AIController->GetPawn();
	if (!AIPawn) return EBTNodeResult::Failed;

	// Get navigation systems in the world
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSystem) return EBTNodeResult::Failed;

	// Variables to receive results from the GetRandomPointInNavigableRadius function
	FNavLocation RandomLocation;

	// Find a valid random location on the NavMesh within a SearchRadius distance around the AIPawn.
	// The result is stored in RandomLocation, and success is returned as bFound.
	bool bFound = NavSystem->GetRandomPointInNavigableRadius(AIPawn->GetActorLocation(), SearchRadius, RandomLocation);
	if (!bFound)
	{
		LOG(TEXT("GetRandomPoint is doesn't exist. bFound is off"));
		return EBTNodeResult::Failed;
	}

	OwnerComp.GetBlackboardComponent()->SetValueAsVector(TargetLocationKey.SelectedKeyName, RandomLocation.Location);
	return EBTNodeResult::Succeeded;
}
