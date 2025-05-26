#include "Boss/Task/Enhanced/BTTask_ChangePerceptionType.h"
#include "Boss/Boss.h"
#include "Boss/Enum/EPerceptionType.h"

UBTTask_ChangePerceptionType::UBTTask_ChangePerceptionType()
{
	NodeName = "Change Perception Type";
	bNotifyTick = true;
	Boss = nullptr;
	AIController = nullptr;
	PerceptionType = EPerceptionType::None;
}

EBTNodeResult::Type UBTTask_ChangePerceptionType::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	Boss = Cast<ABoss>(AIController->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	AIController->SetBlackboardPerceptionType(PerceptionType);	
	
	return EBTNodeResult::Succeeded;
}
