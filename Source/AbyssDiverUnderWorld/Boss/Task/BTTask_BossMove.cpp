#include "Boss/Task/BTTask_BossMove.h"
#include "AbyssDiverUnderWorld.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Boss/Boss.h"
#include "Boss/EBossState.h"
#include "Navigation/PathFollowingComponent.h"

const FName UBTTask_BossMove::BossStateKey = "BossState";

UBTTask_BossMove::UBTTask_BossMove()
{
	NodeName = TEXT("Boss Move");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_BossMove::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 컨트롤러 캐스팅에 실패하면 얼리 리턴
	ABossAIController* AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	// 보스 캐스팅에 실패하면 얼리 리턴
	ABoss* Boss = Cast<ABoss>(AIController->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	// 일반 이동속도로 현재 Patrol Point로 보스 이동
	AIController->MoveToLocationWithRadius(Boss->GetTargetPointLocation());
	Boss->SetMoveSpeed(MoveSpeedMultiplier);

	// Tick Task로 전이
	return EBTNodeResult::InProgress;
}

void UBTTask_BossMove::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	// 캐스팅
	ABossAIController* AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	ABoss* Boss = Cast<ABoss>(AIController->GetCharacter());

	// 현재 Patrol Point 방향으로 보스 회전
	Boss->RotationToTarget(Boss->GetTargetPoint());
	
	// 현재 Patrol Point로 이동 완료한 경우
	// 다음 Patrol Point로 세팅한 후 Idle 상태로 전이
	if (AIController->GetPathFollowingComponent()->GetStatus() == EPathFollowingStatus::Idle)
	{
		Boss->AddPatrolPoint();
		Boss->SetBossState(EBossState::Idle);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
}
