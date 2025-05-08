#include "Boss/Task/BTTask_ChangeBossState.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Boss/EBossState.h"

const FName UBTTask_ChangeBossState::BossStateKey = "BossState";

UBTTask_ChangeBossState::UBTTask_ChangeBossState()
{
	NodeName = TEXT("ChangeBossState");
	BossState = EBossState::Idle;
}

EBTNodeResult::Type UBTTask_ChangeBossState::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    OwnerComp.GetBlackboardComponent()->SetValueAsEnum("BossState", static_cast<uint8>(BossState));

	return EBTNodeResult::Succeeded;
}
