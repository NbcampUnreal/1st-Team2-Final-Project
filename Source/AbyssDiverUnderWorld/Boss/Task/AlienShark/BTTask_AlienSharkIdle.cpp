#include "Boss/Task/AlienShark/BTTask_AlienSharkIdle.h"
#include "Boss/AlienShark/AlienShark.h"

UBTTask_AlienSharkIdle::UBTTask_AlienSharkIdle()
{
	NodeName = "AlienShark Idle";
}

EBTNodeResult::Type UBTTask_AlienSharkIdle::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 캐스팅
	ABossAIController* AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;
	
	AAlienShark* AlienShark = Cast<AAlienShark>(AIController->GetCharacter());
	if (!IsValid(AlienShark)) return EBTNodeResult::Failed;

	// 움직임 정지
	AIController->StopMovement();

	// 다음 Patrol Point 세팅
	AlienShark->AddPatrolPoint();
	
	return EBTNodeResult::Succeeded;
}
