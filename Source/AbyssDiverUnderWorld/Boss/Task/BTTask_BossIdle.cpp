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

	ACharacter* Character = AIController->GetCharacter();
	if (!IsValid(Character)) return EBTNodeResult::Failed;

	ABoss* Boss = Cast<ABoss>(Character);
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	Boss->MoveStop();

	return EBTNodeResult::Succeeded;
}
