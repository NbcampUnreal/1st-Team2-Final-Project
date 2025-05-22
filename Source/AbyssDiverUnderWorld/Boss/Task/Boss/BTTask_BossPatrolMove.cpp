#include "Boss/Task/Boss/BTTask_BossPatrolMove.h"
#include "Boss/Boss.h"
#include "Boss/Enum/EBossState.h"

UBTTask_BossPatrolMove::UBTTask_BossPatrolMove()
{
	NodeName = "Boss Patrol Move";
	bNotifyTick = true;
	MoveSpeedMultiplier = 1.0f;
}

EBTNodeResult::Type UBTTask_BossPatrolMove::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 컨트롤러 캐스팅에 실패하면 얼리 리턴
	ABossAIController* AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	// 보스 캐스팅에 실패하면 얼리 리턴
	ABoss* Boss = Cast<ABoss>(AIController->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	// 다음 이동지점 할당
	PatrolPoint = Boss->GetNextPatrolPoint();

	// 일반 이동속도로 현재 Patrol Point로 보스 이동
	AIController->MoveToLocationWithRadius(PatrolPoint);
	Boss->SetMoveSpeed(MoveSpeedMultiplier);
	
	return EBTNodeResult::InProgress;
}

void UBTTask_BossPatrolMove::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	// 캐스팅
	ABossAIController* AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	ABoss* Boss = Cast<ABoss>(AIController->GetCharacter());
	
	// 현재 Patrol Point로 이동 완료한 경우 Idle 상태로 전이
	if (AIController->GetPathFollowingComponent()->GetStatus() == EPathFollowingStatus::Idle)
	{
		Boss->SetBossState(EBossState::Idle);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
}
