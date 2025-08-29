#include "Monster/BT/Commons/BTTask_ChangePerceptionType.h"

#include "Monster/Boss/Boss.h"
#include "Monster/Boss/Enum/EPerceptionType.h"

UBTTask_ChangePerceptionType::UBTTask_ChangePerceptionType()
{
	NodeName = "Change Perception Type";
	bNotifyTick = false;
	bCreateNodeInstance = false;
	
	PerceptionType = EPerceptionType::None;
}

EBTNodeResult::Type UBTTask_ChangePerceptionType::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	AEnhancedBossAIController* AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	ABoss* Boss = Cast<ABoss>(AIController->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	AIController->SetBlackboardPerceptionType(PerceptionType);	
	
	return EBTNodeResult::Succeeded;
}
