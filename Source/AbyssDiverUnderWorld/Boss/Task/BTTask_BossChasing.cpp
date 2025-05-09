#include "Boss/Task/BTTask_BossChasing.h"
#include "AbyssDiverUnderWorld.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Boss/Boss.h"
#include "Boss/EBossState.h"

const FName UBTTask_BossChasing::BossStateKey = "BossState";

UBTTask_BossChasing::UBTTask_BossChasing()
{
	NodeName = TEXT("Boss Chasing");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_BossChasing::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ABossAIController* AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	ACharacter* Character = AIController->GetCharacter();
	if (!IsValid(Character)) return EBTNodeResult::Failed;

	ABoss* Boss = Cast<ABoss>(Character);
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	AIController->SetChasingVisionAngle();
	Boss->SetMoveSpeed(MoveSpeedMultiplier);
	
	return EBTNodeResult::InProgress;
}

void UBTTask_BossChasing::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
	
	ABoss* Boss = Cast<ABoss>(OwnerComp.GetAIOwner()->GetCharacter());
	if (!IsValid(Boss))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (AccumulatedTime >= TickInterval)
	{
		Boss->MoveToTarget();
		AccumulatedTime = 0.0f;
	}
	else
	{
		AccumulatedTime += DeltaSeconds;
		return;
	}
	
	if (static_cast<EBossState>(OwnerComp.GetBlackboardComponent()->GetValueAsEnum(BossStateKey)) == EBossState::Investigate)
	{
		LOG(TEXT("Chasing Finished !"))
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	
}
