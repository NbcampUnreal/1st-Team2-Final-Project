#include "Boss/Task/Enhanced/BTTask_Damaged.h"
#include "Boss/Boss.h"

UBTTask_Damaged::UBTTask_Damaged()
{
	NodeName = "Damaged";
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	Boss = nullptr;
	AIController = nullptr;
}

EBTNodeResult::Type UBTTask_Damaged::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	Boss = Cast<ABoss>(AIController->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;
	
	return EBTNodeResult::InProgress;
}

void UBTTask_Damaged::TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(Comp, NodeMemory, DeltaSeconds);
}

void UBTTask_Damaged::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	AIController->InitBlackboardVariables();
}
