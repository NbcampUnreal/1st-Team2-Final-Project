// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BT/BTTask_ChasePlayer.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Monster/EMonsterState.h"
#include "Monster/Monster.h"
#include "AbyssDiverUnderWorld.h"

UBTTask_ChasePlayer::UBTTask_ChasePlayer()
{
	NodeName = TEXT("Chase Player");
	bNotifyTaskFinished = true;
	bNotifyTick = true;

	AcceptanceRadius = 300.0f;
}

EBTNodeResult::Type UBTTask_ChasePlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	CachedOwnerComp = &OwnerComp;

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
	if (!BlackboardComp) return EBTNodeResult::Failed;

	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!TargetActor || !IsValid(TargetActor)) return EBTNodeResult::Failed;

	CachedTargetActor = TargetActor;

	// MoveRequest
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(TargetActor);
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
	MoveRequest.SetCanStrafe(false);

	FNavPathSharedPtr NavPath; // Variable to receive the navigation path the AI will actually follow
	FPathFollowingRequestResult Result = AIController->MoveTo(MoveRequest, &NavPath);

	if (Result.Code == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		// Already Arrived -> State Change
		BlackboardComp->SetValueAsEnum(MonsterStateKey.SelectedKeyName, static_cast<uint8>(EMonsterState::Attack));
		return EBTNodeResult::Succeeded;
	}
	else if (Result.Code == EPathFollowingRequestResult::RequestSuccessful)
	{
		// MonsterState : Chase
		return EBTNodeResult::InProgress;
	}
	else
	{
		return EBTNodeResult::Failed;
	}
}

void UBTTask_ChasePlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return;

	ACharacter* AIPawn = Cast<ACharacter>(AIController->GetPawn());
	if (!AIPawn) return;

	UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
	if (!BlackboardComp) return;

	if (!CachedTargetActor || !IsValid(CachedTargetActor))
	{
		// Lost TargetActor
		FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Failed);

		// MonsterAIController handles this processing, but explicitly writes
		BlackboardComp->SetValueAsEnum(MonsterStateKey.SelectedKeyName, static_cast<uint8>(EMonsterState::Patrol));
		BlackboardComp->ClearValue(TargetActorKey.SelectedKeyName);
		return;
	}

	if (AIController->GetMoveStatus() == EPathFollowingStatus::Idle)
	{
		// When it arrives, switch the status to Attack
		BlackboardComp->SetValueAsEnum(MonsterStateKey.SelectedKeyName, static_cast<uint8>(EMonsterState::Attack));
		FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
	}
}

