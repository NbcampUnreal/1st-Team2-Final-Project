#include "Monster/Boss/Task/AlienShark/BTTask_PerformAttack.h"
#include "AIController.h"
#include "Monster/Boss/Boss.h"
#include "Monster/Boss/EnhancedBossAIController.h"

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
	TaskMemory->Boss = Cast<ABoss>(Comp.GetAIOwner()->GetCharacter());

	if (!TaskMemory->Boss.IsValid() || !TaskMemory->AIController.IsValid()) return EBTNodeResult::Failed;

	TaskMemory->Boss->Attack();
	TaskMemory->AIController->GetBlackboardComponent()->SetValueAsBool(bCanAttackKey, false);
	
	return EBTNodeResult::Succeeded;
}

