#include "Monster/Boss/Task/AlienShark/BTTask_StopMovement.h"
#include "AIController.h"
#include "Monster/Boss/Boss.h"
#include "Monster/AquaticMovementComponent.h"

UBTTask_StopMovement::UBTTask_StopMovement()
{
	NodeName = TEXT("Stop Movement");
}

EBTNodeResult::Type UBTTask_StopMovement::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	// 기존 AIController의 이동 정지
	Comp.GetAIOwner()->StopMovement();
	
	// AquaticMovementComponent의 이동도 정지
	ABoss* Boss = Cast<ABoss>(Comp.GetAIOwner()->GetCharacter());
	if (Boss && Boss->AquaticMovementComponent)
	{
		Boss->AquaticMovementComponent->StopMovement();
	}
	
	return EBTNodeResult::Succeeded;
}
