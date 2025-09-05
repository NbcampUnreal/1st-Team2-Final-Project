// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BT/Commons/BTTask_ChaseByState.h"

#include "Container/BlackboardKeys.h"

#include "Monster/Monster.h"
#include "Monster/MonsterAIController.h"

#include "Character/UnderwaterCharacter.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"

UBTTask_ChaseByState::UBTTask_ChaseByState()
{
	NodeName = TEXT("ChaseByState");
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBTTask_ChaseByState::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTChaseByStateTaskMemory* TaskMemory = (FBTChaseByStateTaskMemory*)NodeMemory;
	if (!TaskMemory) return EBTNodeResult::Failed;

	TaskMemory->AIController = Cast<AMonsterAIController>(OwnerComp.GetAIOwner());
	TaskMemory->Monster = Cast<AMonster>(OwnerComp.GetAIOwner()->GetCharacter());

	if (!TaskMemory->AIController.IsValid() || !TaskMemory->Monster.IsValid()) return EBTNodeResult::Failed;

	// Default : 10 ~ 20초 사이
	TaskMemory->ChasingTime = FMath::RandRange(MinChasingTime, MaxChasingTime);

	return EBTNodeResult::InProgress;
}

void UBTTask_ChaseByState::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	FBTChaseByStateTaskMemory* TaskMemory = (FBTChaseByStateTaskMemory*)NodeMemory;
	if (!TaskMemory) return;

	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(TaskMemory->AIController->GetBlackboardComponent()->GetValueAsObject(BlackboardKeys::TargetPlayerKey));

	// 타겟 플레이어가 유효하지 않는 경우
	if (!IsValid(Player))
	{
		TaskMemory->Monster->RemoveDetection(Player);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// 추적 시간이 경과한 경우
	TaskMemory->AccumulatedTime += DeltaSeconds;
	if (TaskMemory->AccumulatedTime > TaskMemory->ChasingTime)
	{
		TaskMemory->Monster->RemoveDetection(Player);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// 공격 영역 내에 들어온 경우
	if (!TaskMemory->bHasAttacked && TaskMemory->Monster->GetIsAttackCollisionOverlappedPlayer())
	{
		TaskMemory->bHasAttacked = true;
		TaskMemory->AIController->GetBlackboardComponent()->SetValueAsBool(BlackboardKeys::bCanAttackKey, true);
	}

	// 추적 중인 플레이어가 시야에서 벗어났는지 확인.
	// 플레이어가 시야에서 벗어난 상태에서 해초에 숨었다면
	// 플레이어 주변의 가장 가까운 점으로 이동한다. (EQS)
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
				TaskMemory->AIController->GetBlackboardComponent()->SetValueAsBool(BlackboardKeys::bIsPlayerHiddenKey, true);
				TaskMemory->Monster->RemoveDetection(Player);
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
				return;
			}
		}
	}

	// PerformChasing 메소드에서 TargetLocation 계속 Update, 이동 및 회전 처리는 AquaticMovementComponent
	TaskMemory->Monster->PerformChasing(DeltaSeconds);
}

void UBTTask_ChaseByState::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	FBTChaseByStateTaskMemory* TaskMemory = (FBTChaseByStateTaskMemory*)NodeMemory;
	if (!TaskMemory) return;

	TaskMemory->AccumulatedTime = 0.0f;
	TaskMemory->bHasAttacked = false;
}
