#include "Monster/BT/Commons/BTTask_ChangePerceptionType.h"

#include "Monster/Monster.h"
#include "Monster/EPerceptionType.h"
#include "Monster/MonsterAIController.h"

UBTTask_ChangePerceptionType::UBTTask_ChangePerceptionType()
{
	NodeName = "Change Perception Type";
	bNotifyTick = false;
	bCreateNodeInstance = false;
	
	PerceptionType = EPerceptionType::None;
}

EBTNodeResult::Type UBTTask_ChangePerceptionType::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	AMonsterAIController* AIController = Cast<AMonsterAIController>(Comp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	AMonster* Monster = Cast<AMonster>(AIController->GetCharacter());
	if (!IsValid(Monster)) return EBTNodeResult::Failed;

	AIController->SetBlackboardPerceptionType(PerceptionType);	
	
	return EBTNodeResult::Succeeded;
}
