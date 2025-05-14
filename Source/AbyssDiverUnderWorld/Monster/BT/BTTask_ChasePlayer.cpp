// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BT/BTTask_ChasePlayer.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Monster/FlyingAIPathfindingBase.h"
#include "AbyssDiverUnderWorld.h"

UBTTask_ChasePlayer::UBTTask_ChasePlayer()
{
	NodeName = TEXT("Chase Player");
	bNotifyTaskFinished = true;
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_ChasePlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	CachedOwnerComp = OwnerComp;

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	APawn* AIPawn = AIController->GetPawn();
	if (!AIPawn) return EBTNodeResult::Failed;

	CachedPathComp = AIPawn->FindComponentByClass<UFlyingAIPathfindingBase>();
	if (!CachedPathComp) return EBTNodeResult::Failed;

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp) return EBTNodeResult::Failed;

	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject("TargetActor"));
	if (!TargetActor || !IsValid(TargetActor)) return EBTNodeResult::Failed;

	CachedTargetActor = TargetActor;

	// Arrived Event Binding
	CachedPathComp->OnFinishedMoving.AddDynamic(this, &UBTTask_ChasePlayer::HandleMoveFinishied);

	if (AIPawn->HasAuthority())
	{
		CachedPathComp->S_MoveTo(TargetActor->GetActorLocation());
	}
	else
	{
		return EBTNodeResult::Failed;
	}
	
	return EBTNodeResult::InProgress;
}

void UBTTask_ChasePlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (!CachedTargetActor || !IsValid(CachedTargetActor))
	{
		FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Failed);
		LOG(TEXT("TargetActor is Invalid"));
	}
}

void UBTTask_ChasePlayer::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	if (CachedPathComp)
	{
		CachedPathComp->OnFinishedMoving.RemoveDynamic(this, &UBTTask_ChasePlayer::HandleMoveFinishied);
	}
}

void UBTTask_ChasePlayer::HandleMoveFinishied()
{
	if (CachedOwnerComp)
	{
		FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
	}

	if (CachedPathComp)
	{
		CachedPathComp->OnFinishedMoving.RemoveDynamic(this, &UBTTask_ChasePlayer::HandleMoveFinishied);
	}
}
