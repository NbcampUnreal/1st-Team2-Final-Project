#include "Boss/Task/Enhanced/BTTask_EnhancedIdle.h"
#include "AbyssDiverUnderWorld.h"
#include "Boss/Boss.h"
#include "Boss/EnhancedBossAIController.h"

UBTTask_EnhancedIdle::UBTTask_EnhancedIdle()
{
	NodeName = "Enhanced Idle";
	Boss = nullptr;
	AIController = nullptr;
}

EBTNodeResult::Type UBTTask_EnhancedIdle::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	Boss = Cast<ABoss>(AIController->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	AIController->StopMovement();
	
	return EBTNodeResult::Succeeded;
}
