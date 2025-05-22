#include "Boss/Task/Enhanced/BTTask_BloodChase.h"
#include "Boss/Boss.h"

UBTTask_BloodChase::UBTTask_BloodChase()
{
	NodeName = "Blood Chase";
	bNotifyTick = true;
	Boss = nullptr;
	AIController = nullptr;
}

EBTNodeResult::Type UBTTask_BloodChase::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	Boss = Cast<ABoss>(AIController->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;
	
	return EBTNodeResult::InProgress;
}

void UBTTask_BloodChase::TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(Comp, NodeMemory, DeltaSeconds);
}
