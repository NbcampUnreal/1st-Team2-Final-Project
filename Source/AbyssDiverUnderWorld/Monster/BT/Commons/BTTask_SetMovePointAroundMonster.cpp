#include "Monster/BT/Commons/BTTask_SetMovePointAroundMonster.h"

#include "Monster/Boss/Boss.h"

// ----- 기능 -----
// 1. AI 주변의 이동 가능한 위치 추출
// 2. 해당 위치를 Blackboard에 저장

// ----- 사용처 -----
// Idle 상태 이후 특정 위치로 이동할 때

UBTTask_SetMovePointAroundMonster::UBTTask_SetMovePointAroundMonster()
{
	NodeName = "Set Move Point Around Monster";
}

EBTNodeResult::Type UBTTask_SetMovePointAroundMonster::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	AEnhancedBossAIController* AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;
	
	ABoss* Boss = Cast<ABoss>(AIController->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	const FName BlackboardKeyName = GetSelectedBlackboardKey();
	AIController->GetBlackboardComponent()->SetValueAsVector(BlackboardKeyName, Boss->GetNextPatrolPoint());
	
	return EBTNodeResult::Succeeded;
}
