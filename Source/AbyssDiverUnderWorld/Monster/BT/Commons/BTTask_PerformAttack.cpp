#include "Monster/BT/Commons/BTTask_PerformAttack.h"

#include "Monster/Monster.h"
#include "Monster/Boss/EnhancedBossAIController.h"

#include "AIController.h"

const FName UBTTask_PerformAttack::bCanAttackKey = "bCanAttack";

UBTTask_PerformAttack::UBTTask_PerformAttack()
{
	NodeName = TEXT("Perform Attack");
	bNotifyTick = false;
	bNotifyTaskFinished = false;
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBTTask_PerformAttack::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	FBTPerformAttackMemory* TaskMemory = (FBTPerformAttackMemory*) NodeMemory;
	if (!TaskMemory) return EBTNodeResult::Failed;

	TaskMemory->AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	TaskMemory->Monster = Cast<AMonster>(Comp.GetAIOwner()->GetCharacter());

	if (!TaskMemory->Monster.IsValid() || !TaskMemory->AIController.IsValid()) return EBTNodeResult::Failed;

	TaskMemory->Monster->Attack();
	TaskMemory->AIController->GetBlackboardComponent()->SetValueAsBool(bCanAttackKey, false);
	
	return EBTNodeResult::Succeeded;
}

