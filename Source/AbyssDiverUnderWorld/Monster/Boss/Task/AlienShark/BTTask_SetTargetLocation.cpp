#include "Monster/Boss/Task/AlienShark/BTTask_SetTargetLocation.h"
#include "Monster/Boss/Boss.h"
#include "Monster/Boss/EnhancedBossAIController.h"

UBTTask_SetTargetLocation::UBTTask_SetTargetLocation()
{
	NodeName = TEXT("Set TargetLocation");
	bNotifyTick = false;
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBTTask_SetTargetLocation::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	AEnhancedBossAIController* AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	ABoss* Boss = Cast<ABoss>(Comp.GetAIOwner()->GetCharacter());
	
	if (!IsValid(AIController) | !IsValid(Boss)) return EBTNodeResult::Failed;

	const FName BlackboardKeyName = GetSelectedBlackboardKey();
	const FVector TargetLocation = AIController->GetBlackboardComponent()->GetValueAsVector(BlackboardKeyName);

	Boss->SetTargetLocation(TargetLocation);

	AIController->GetBlackboardComponent()->SetValueAsObject("TargetPlayer", nullptr);
	
	return EBTNodeResult::Succeeded;
}
