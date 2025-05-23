#include "Boss/Task/Enhanced/BTTask_HiddenPlayerChase.h"
#include "Boss/Boss.h"
#include "Boss/ENum/EBossState.h"

UBTTask_HiddenPlayerChase::UBTTask_HiddenPlayerChase()
{
	NodeName = "Hidden Player Chase";
	bNotifyTick = true;
	Boss = nullptr;
	AIController = nullptr;
}

EBTNodeResult::Type UBTTask_HiddenPlayerChase::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	Boss = Cast<ABoss>(AIController->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	Boss->SetBossState(EBossState::Chase);
	
	return EBTNodeResult::InProgress;
}

void UBTTask_HiddenPlayerChase::TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(Comp, NodeMemory, DeltaSeconds);
}
