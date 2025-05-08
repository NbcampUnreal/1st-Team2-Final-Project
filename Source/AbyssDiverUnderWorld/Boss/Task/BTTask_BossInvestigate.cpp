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

	ACharacter* Character = AIController->GetCharacter();
	if (!IsValid(Character)) return EBTNodeResult::Failed;

	ABoss* Boss = Cast<ABoss>(Character);
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	AIController->SetDefaultVisionAngle();
	Boss->MoveToLastDetectedLocation();

	LOG(TEXT("Investigate Started !"));

	return EBTNodeResult::Succeeded;
}
