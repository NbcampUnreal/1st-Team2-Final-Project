#include "Monster/BT/Commons/BTTask_PerformNormalMovement.h"

#include "Monster/Monster.h"
#include "Monster/Boss/EnhancedBossAIController.h"

#include "AIController.h"

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

	TaskMemory->AIController = Cast<AMonsterAIController>(OwnerComp.GetAIOwner());
	TaskMemory->Monster = Cast<AMonster>(OwnerComp.GetAIOwner()->GetCharacter());

	if (!TaskMemory->AIController.IsValid() || !TaskMemory->Monster.IsValid()) return EBTNodeResult::Failed;
	
	return EBTNodeResult::InProgress;
}

void UBTTask_PerformNormalMovement::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	FBTPerformNormalMovementMemory* TaskMemory = (FBTPerformNormalMovementMemory*)NodeMemory;
	if (!TaskMemory) return;

	TaskMemory->Monster->PerformNormalMovement(DeltaSeconds);
}
