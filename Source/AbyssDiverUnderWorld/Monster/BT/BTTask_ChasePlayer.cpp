// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BT/BTTask_ChasePlayer.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Monster/EMonsterState.h"
#include "Character/UnderwaterCharacter.h"
#include "Monster/Monster.h"
#include "AbyssDiverUnderWorld.h"

const FName UBTTask_ChasePlayer::bCanAttackKey = "bCanAttack";
const FName UBTTask_ChasePlayer::bIsHidingKey = "bIsHiding";
const FName UBTTask_ChasePlayer::bIsPlayerHiddenKey = "bIsPlayerHidden";
const FName UBTTask_ChasePlayer::TargetPlayerKey = "TargetPlayer";

UBTTask_ChasePlayer::UBTTask_ChasePlayer()
{
	NodeName = TEXT("Chase Player");
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBTTask_ChasePlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTChasePlayerTaskMemory* TaskMemory = (FBTChasePlayerTaskMemory*)NodeMemory;
	if (!TaskMemory) return EBTNodeResult::Failed;

	TaskMemory->AIController = Cast<AMonsterAIController>(OwnerComp.GetAIOwner());
	TaskMemory->Monster = Cast<AMonster>(OwnerComp.GetAIOwner()->GetCharacter());

	if (!TaskMemory->AIController.IsValid() || !TaskMemory->Monster.IsValid()) return EBTNodeResult::Failed;

	TaskMemory->ChasingTime = FMath::RandRange(MinChasingTime, MaxChasingTime);

	return EBTNodeResult::InProgress;
}

void UBTTask_ChasePlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	FBTChasePlayerTaskMemory* TaskMemory = (FBTChasePlayerTaskMemory*)NodeMemory;
	if (!TaskMemory) return;

	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(TaskMemory->AIController->GetBlackboardComponent()->GetValueAsObject("TargetPlayer"));

	if (!IsValid(Player))
	{
		LOG(TEXT("ChasePlayer: Player is not valid"));
		TaskMemory->AIController->GetBlackboardComponent()->SetValueAsBool(bIsHidingKey, true);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	TaskMemory->AccumulatedTime += DeltaSeconds;
	if (TaskMemory->AccumulatedTime > TaskMemory->ChasingTime)
	{
		LOG(TEXT("PerformChasing: Chasing time exceeded"));
		TaskMemory->AIController->GetBlackboardComponent()->SetValueAsBool(bIsHidingKey, true);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	if (!bHasAttacked && TaskMemory->Monster->GetIsAttackCollisionOverlappedPlayer())
	{
		LOG(TEXT("PerformChasing: Player Is Overlapped With Attack Collision"));
		bHasAttacked = true;
		TaskMemory->AIController->GetBlackboardComponent()->SetValueAsBool(bCanAttackKey, true);
	}

	FActorPerceptionBlueprintInfo PerceptionInfo;
	const bool bIsPerceptionSuccess = TaskMemory->AIController->PerceptionComponent->GetActorsPerception(Player, PerceptionInfo);
	if (bIsPerceptionSuccess)
	{
		bool bIsPlayerInSight = false;

		for (const FAIStimulus& Stimulus : PerceptionInfo.LastSensedStimuli)
		{
			if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>() &&
				Stimulus.WasSuccessfullySensed())
			{
				bIsPlayerInSight = true;
				break;
			}
		}

		if (!bIsPlayerInSight)
		{
			if (Player->IsHideInSeaweed())
			{
				TaskMemory->AIController->GetBlackboardComponent()->SetValueAsBool(bIsPlayerHiddenKey, true);
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
				return;
			}
		}
	}

	TaskMemory->Monster->PerformChasing(DeltaSeconds);
}

void UBTTask_ChasePlayer::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	FBTChasePlayerTaskMemory* TaskMemory = (FBTChasePlayerTaskMemory*)NodeMemory;
	if (!TaskMemory) return;

	TaskMemory->AccumulatedTime = 0.0f;
	TaskMemory->Monster->InitTarget();
	bHasAttacked = false;
}

