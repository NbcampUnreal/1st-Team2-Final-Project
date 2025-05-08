#include "Boss/Task/BTTask_BossMove.h"
#include "AIController.h"
#include "Boss/Boss.h"

UBTTask_BossMove::UBTTask_BossMove()
{
	NodeName = TEXT("Boss Move");
}

EBTNodeResult::Type UBTTask_BossMove::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	ACharacter* Character = AIController->GetCharacter();
	if (!IsValid(Character)) return EBTNodeResult::Failed;

	ABoss* Boss = Cast<ABoss>(Character);
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	Boss->Move();
	Boss->SetMoveSpeed(MoveSpeedMultiplier);

	return EBTNodeResult::Succeeded;
}
