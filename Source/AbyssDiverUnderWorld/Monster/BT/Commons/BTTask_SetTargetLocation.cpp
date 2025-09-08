#include "Monster/BT/Commons/BTTask_SetTargetLocation.h"

#include "Monster/Monster.h"
#include "Monster/MonsterAIController.h"
#include "Monster/Components/AquaticMovementComponent.h"

UBTTask_SetTargetLocation::UBTTask_SetTargetLocation()
{
	NodeName = TEXT("Set TargetLocation");
	bNotifyTick = false;
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBTTask_SetTargetLocation::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	AMonsterAIController* AIController = Cast<AMonsterAIController>(Comp.GetAIOwner());
	AMonster* Monster = Cast<AMonster>(Comp.GetAIOwner()->GetCharacter());
	
	if (!IsValid(AIController) || !IsValid(Monster)) return EBTNodeResult::Failed;

	const FName BlackboardKeyName = GetSelectedBlackboardKey();
	const FVector TargetLocation = AIController->GetBlackboardComponent()->GetValueAsVector(BlackboardKeyName);

	Monster->AquaticMovementComponent->SetTargetLocation(TargetLocation);

	//AIController->GetBlackboardComponent()->SetValueAsObject("TargetPlayer", nullptr);
	
	return EBTNodeResult::Succeeded;
}
