#include "Boss/Task/BTTask_BossIdle.h"
#include "AIController.h"
#include "Boss/Boss.h"

UBTTask_BossIdle::UBTTask_BossIdle()
{
	NodeName = TEXT("Boss Idle");
}

EBTNodeResult::Type UBTTask_BossIdle::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	ABoss* Boss = Cast<ABoss>(AIController->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	Boss->MoveStop();

	return EBTNodeResult::Succeeded;
}
