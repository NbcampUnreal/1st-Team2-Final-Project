#include "Monster/Boss/Task/AlienShark/BTTask_SetBlackboardKey.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_SetBlackboardKey::UBTTask_SetBlackboardKey()
{
	NodeName = TEXT("Set BlackboardKey");
	bNotifyTick = false;
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBTTask_SetBlackboardKey::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	//AEnhancedBossAIController* AIController = Cast<AEnhancedBossAIController>();
	//if (!IsValid(AIController)) return EBTNodeResult::Failed;

	const FName BlackboardKeyName = GetSelectedBlackboardKey();
	Comp.GetAIOwner()->GetBlackboardComponent()->SetValueAsBool(BlackboardKeyName, BlackBoardKeyValue);
	
	return EBTNodeResult::Succeeded;
}
