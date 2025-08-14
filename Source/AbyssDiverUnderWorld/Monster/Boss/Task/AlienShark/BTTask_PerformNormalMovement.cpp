#include "Monster/Boss/Task/AlienShark/BTTask_PerformNormalMovement.h"

#include "AIController.h"
#include "Monster/Boss/Boss.h"
#include "Monster/Boss/EnhancedBossAIController.h"

UBTTask_PerformNormalMovement::UBTTask_PerformNormalMovement()
{
	NodeName = TEXT("Perform Normal Movement");
	bNotifyTick = true;
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBTTask_PerformNormalMovement::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTPerformNormalMovementMemory* TaskMemory = (FBTPerformNormalMovementMemory*)NodeMemory;
	if (!TaskMemory) return EBTNodeResult::Failed;

	TaskMemory->AIController = Cast<AEnhancedBossAIController>(OwnerComp.GetAIOwner());
	TaskMemory->Boss = Cast<ABoss>(OwnerComp.GetAIOwner()->GetCharacter());

	if (!TaskMemory->AIController.IsValid() || !TaskMemory->Boss.IsValid()) return EBTNodeResult::Failed;
	
	return EBTNodeResult::InProgress;
}

void UBTTask_PerformNormalMovement::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	FBTPerformNormalMovementMemory* TaskMemory = (FBTPerformNormalMovementMemory*)NodeMemory;
	if (!TaskMemory) return;

	TaskMemory->Boss->PerformNormalMovement(DeltaSeconds);
}
