#include "Boss/Task/Enhanced/BTTask_EnhancedIdle.h"
#include "AbyssDiverUnderWorld.h"
#include "Boss/Boss.h"
#include "Boss/EnhancedBossAIController.h"
#include "Boss/ENum/EBossState.h"

UBTTask_EnhancedIdle::UBTTask_EnhancedIdle()
{
	NodeName = "Enhanced Idle";
	bNotifyTick = true;
	Boss = nullptr;
	AIController = nullptr;
	IdleFinishMaxInterval = 5.0f;
	IdleFinishMinInterval = 2.0f;
}

EBTNodeResult::Type UBTTask_EnhancedIdle::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	Boss = Cast<ABoss>(AIController->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	Boss->SetBossState(EBossState::Idle);

	AccumulatedTime = 0.f;
	IdleFinishTime = FMath::RandRange(IdleFinishMinInterval, IdleFinishMaxInterval);
	
	return EBTNodeResult::InProgress;
}

void UBTTask_EnhancedIdle::TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(Comp, NodeMemory, DeltaSeconds);

	Boss->InitializeRotation(DeltaSeconds);

	if (AccumulatedTime > IdleFinishTime)
	{
		FinishLatentTask(Comp, EBTNodeResult::Succeeded);
	}
	
	AccumulatedTime += FMath::Clamp(DeltaSeconds, 0.f, 1.f);
}
