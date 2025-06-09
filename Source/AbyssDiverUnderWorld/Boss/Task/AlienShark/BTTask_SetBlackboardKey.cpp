#include "Boss/Task/AlienShark/BTTask_SetBlackboardKey.h"
#include "Boss/EnhancedBossAIController.h"

UBTTask_SetBlackboardKey::UBTTask_SetBlackboardKey()
{
	NodeName = TEXT("Set BlackboardKey");
	bNotifyTick = false;
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBTTask_SetBlackboardKey::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	AEnhancedBossAIController* AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	const FName BlackboardKeyName = GetSelectedBlackboardKey();
	AIController->GetBlackboardComponent()->SetValueAsBool(BlackboardKeyName, BlackBoardKeyValue);
	
	return EBTNodeResult::Succeeded;
}
