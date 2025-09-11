#include "Monster/BT/Commons/BTTaskNode_StopImmediately.h"

#include "AbyssDiverUnderWorld.h"

#include "Monster/Monster.h"
#include "Monster/Components/AquaticMovementComponent.h"	

EBTNodeResult::Type UBTTaskNode_StopImmediately::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	AMonster* Monster = Cast<AMonster>(Comp.GetAIOwner()->GetCharacter());
	if (Monster == nullptr || Monster->AquaticMovementComponent == nullptr)
	{
		LOGV(Error, TEXT("UBTTaskNode_StopImmediately::ExecuteTask : Monster or AquaticMovementComponent is nullptr"));
		return EBTNodeResult::Failed;
	}

	Monster->AquaticMovementComponent->StopMovementImmediately();
	return EBTNodeResult::Succeeded;
}
