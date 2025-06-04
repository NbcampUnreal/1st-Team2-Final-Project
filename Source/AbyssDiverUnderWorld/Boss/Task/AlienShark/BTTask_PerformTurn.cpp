#include "Boss/Task/AlienShark/BTTask_PerformTurn.h"
#include "Boss/Boss.h"
#include "Boss/EnhancedBossAIController.h"

UBTTask_PerformTurn::UBTTask_PerformTurn()
{
	NodeName = TEXT("Perform Turn");
	bCreateNodeInstance = false;
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_PerformTurn::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTPerformTurnMemory* TaskMemory = (FBTPerformTurnMemory*)NodeMemory;
	if (!TaskMemory) return EBTNodeResult::Failed;
	
	TaskMemory->AIController = Cast<AEnhancedBossAIController>(OwnerComp.GetAIOwner());
	TaskMemory->Boss = Cast<ABoss>(OwnerComp.GetAIOwner()->GetCharacter());
	
	if (!TaskMemory->AIController.IsValid() || !TaskMemory->Boss.IsValid()){return EBTNodeResult::Failed;}
	
	return EBTNodeResult::InProgress;
}

void UBTTask_PerformTurn::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	FBTPerformTurnMemory* TaskMemory = (FBTPerformTurnMemory*)NodeMemory;
	if (!TaskMemory) return;

	TaskMemory->Boss->PerformTurn(DeltaSeconds);
}
