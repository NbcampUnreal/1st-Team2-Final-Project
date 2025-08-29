#include "Monster/BT/AlienShark/BTTask_StopMovement.h"

#include "Monster/Monster.h"
#include "Monster/Components/AquaticMovementComponent.h"

#include "AIController.h"

UBTTask_StopMovement::UBTTask_StopMovement()
{
	NodeName = TEXT("Stop Movement");
}

EBTNodeResult::Type UBTTask_StopMovement::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	// 기존 AIController의 이동 정지
	Comp.GetAIOwner()->StopMovement();
	
	// AquaticMovementComponent의 이동도 정지
	AMonster* Monster = Cast<AMonster>(Comp.GetAIOwner()->GetCharacter());
	if (Monster && Monster->AquaticMovementComponent)
	{
		Monster->AquaticMovementComponent->StopMovement();
	}
	
	return EBTNodeResult::Succeeded;
}
