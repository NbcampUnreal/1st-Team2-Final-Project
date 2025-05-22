#include "Boss/Task/Enhanced/BTTask_EnhancedMove.h"

#include "AbyssDiverUnderWorld.h"
#include "Boss/Boss.h"

UBTTask_EnhancedMove::UBTTask_EnhancedMove()
{
	NodeName = "Enhanced Move";
	bNotifyTick = true;
	Boss = nullptr;
	AIController = nullptr;
}

EBTNodeResult::Type UBTTask_EnhancedMove::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	Boss = Cast<ABoss>(AIController->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	// 다음 이동지점 할당
	PatrolPoint = Boss->GetNextPatrolPoint();

	// 일반 이동속도로 현재 Patrol Point로 보스 이동
	AIController->MoveToLocationWithRadius(PatrolPoint);
	Boss->SetMoveSpeed(MoveSpeedMultiplier);
	
	return EBTNodeResult::InProgress;
}

void UBTTask_EnhancedMove::TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(Comp, NodeMemory, DeltaSeconds);

	// 현재 Patrol Point로 이동 완료 후 Success 반환
	if (AIController->GetPathFollowingComponent()->GetStatus() == EPathFollowingStatus::Idle)
	{
		FinishLatentTask(Comp, EBTNodeResult::Succeeded);
		return;
	}

	if (AIController->GetPathFollowingComponent()->GetStatus() == EPathFollowingResult::Aborted)
	{
		LOG(TEXT("Aborted !"));
	}

	if (AIController->GetPathFollowingComponent()->GetStatus() == EPathFollowingResult::Blocked)
	{
		LOG(TEXT("Blocked !"));
	}

	if (AIController->GetPathFollowingComponent()->GetStatus() == EPathFollowingResult::Invalid)
	{
		LOG(TEXT("Invalid !"));
	}
	
}
