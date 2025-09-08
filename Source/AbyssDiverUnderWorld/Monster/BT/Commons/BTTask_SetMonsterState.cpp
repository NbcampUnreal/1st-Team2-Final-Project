#include "Monster/BT/Commons/BTTask_SetMonsterState.h"

#include "AbyssDiverUnderWorld.h"

#include "Monster/Monster.h"

//#include "BehaviorTree/BlackboardComponent.h"

EBTNodeResult::Type UBTTask_SetMonsterState::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AMonster* Monster = Cast<AMonster>(OwnerComp.GetAIOwner()->GetPawn());
	if (Monster)
	{
		Monster->ApplyMonsterStateChange(NewState);
		return EBTNodeResult::Succeeded;
	}

	LOGV(Warning, TEXT("UBTTask_SetMonsterState::ExecuteTask - Failed to cast to AMonster."));
	return EBTNodeResult::Failed;
}
