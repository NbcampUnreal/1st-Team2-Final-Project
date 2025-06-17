#include "Boss/Task/AlienShark/BTTask_StopMovement.h"
#include "AIController.h"

UBTTask_StopMovement::UBTTask_StopMovement()
{
	NodeName = TEXT("Stop Movement");
}

EBTNodeResult::Type UBTTask_StopMovement::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	Comp.GetAIOwner()->StopMovement();
	
	return EBTNodeResult::Succeeded;
}
