#include "Boss/Task/BTTask_BossInvestigate.h"
#include "AbyssDiverUnderWorld.h"
#include "AIController.h"
#include "Boss/Boss.h"

UBTTask_BossInvestigate::UBTTask_BossInvestigate()
{
	NodeName = TEXT("Boss Investigate");
}

EBTNodeResult::Type UBTTask_BossInvestigate::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ABossAIController* AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	ABoss* Boss = Cast<ABoss>(AIController->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	AIController->SetVisionAngle(AIController->DefaultVisionAngle);
	Boss->MoveToLastDetectedLocation();
	
	return EBTNodeResult::Succeeded;
}
