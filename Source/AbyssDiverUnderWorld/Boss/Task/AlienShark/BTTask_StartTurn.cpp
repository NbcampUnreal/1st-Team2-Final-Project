#include "Boss/Task/AlienShark/BTTask_StartTurn.h"
#include "AIController.h"
#include "Boss/Boss.h"

UBTTask_StartTurn::UBTTask_StartTurn()
{
	NodeName = TEXT("Start Turn");
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBTTask_StartTurn::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTStartTurnMemory* TaskMemory = (FBTStartTurnMemory*)NodeMemory;
	if (!TaskMemory) return EBTNodeResult::Failed;

	TaskMemory->Boss = Cast<ABoss>(OwnerComp.GetAIOwner()->GetCharacter());
	if (!TaskMemory->Boss.IsValid()) return EBTNodeResult::Failed;

	TaskMemory->Boss->StartTurn();
	
	return Super::ExecuteTask(OwnerComp, NodeMemory);
}
