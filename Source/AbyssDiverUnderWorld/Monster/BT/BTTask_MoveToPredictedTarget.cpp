// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BT/BTTask_MoveToPredictedTarget.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_MoveToPredictedTarget::UBTTask_MoveToPredictedTarget()
{
	NodeName = TEXT("Tick MoveTo Predicted Target");
	bNotifyTick = true;
	PredictTime = 0.5f;
	AcceptanceRadius = 100.f;
	MoveUpdateThreshold = 100.f;
	
}

EBTNodeResult::Type UBTTask_MoveToPredictedTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return EBTNodeResult::InProgress;
}

void UBTTask_MoveToPredictedTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!AIController || !BB)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	ACharacter* AIPawn = Cast<ACharacter>(AIController->GetPawn());
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!AIPawn || !Target)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Calculating predicted locations
	const FVector PredictedLocation = Target->GetActorLocation() + Target->GetVelocity() * PredictTime;
	const FVector CurrentLocation = AIPawn->GetActorLocation();

	// Arrival check
	if (FVector::Dist(CurrentLocation, PredictedLocation) <= AcceptanceRadius)
	{
		AIController->StopMovement();
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// MoveTo
	const FVector CurrentGoal = AIController->GetImmediateMoveDestination();
	if (FVector::Dist(CurrentGoal, PredictedLocation) > MoveUpdateThreshold)
	{
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalLocation(PredictedLocation);
		MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
		MoveRequest.SetUsePathfinding(true);
		MoveRequest.SetAllowPartialPath(true);

		FNavPathSharedPtr NavPath;
		AIController->MoveTo(MoveRequest, &NavPath);
	}
}
