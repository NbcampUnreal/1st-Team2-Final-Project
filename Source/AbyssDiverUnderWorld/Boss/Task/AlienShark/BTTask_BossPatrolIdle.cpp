#include "Boss/Task/AlienShark/BTTask_BossPatrolIdle.h"
#include "Boss/Boss.h"

UBTTask_BossPatrolIdle::UBTTask_BossPatrolIdle()
{
	NodeName = "BossPatrol Idle";
}

EBTNodeResult::Type UBTTask_BossPatrolIdle::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 캐스팅
	ABossAIController* AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;
	
	ABoss* Boss = Cast<ABoss>(AIController->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	// 움직임 정지
	AIController->StopMovement();

	// 다음 Patrol Point 세팅
	Boss->AddPatrolPoint();
	
	return EBTNodeResult::Succeeded;
}
