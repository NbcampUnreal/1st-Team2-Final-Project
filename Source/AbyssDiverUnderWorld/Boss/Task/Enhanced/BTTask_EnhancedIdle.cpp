#include "Boss/Task/Enhanced/BTTask_EnhancedIdle.h"
#include "AbyssDiverUnderWorld.h"
#include "Boss/Boss.h"
#include "Boss/EnhancedBossAIController.h"
#include "Boss/ENum/EBossState.h"

UBTTask_EnhancedIdle::UBTTask_EnhancedIdle()
{
	NodeName = "Enhanced Idle";
	bNotifyTick = true;
	bCreateNodeInstance = false;
	
	IdleFinishMaxInterval = 5.0f;
	IdleFinishMinInterval = 2.0f;
}

EBTNodeResult::Type UBTTask_EnhancedIdle::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	FBTIdleTaskMemory* TaskMemory = (FBTIdleTaskMemory*)NodeMemory;
	if (!TaskMemory) return EBTNodeResult::Failed;

	TaskMemory->AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	TaskMemory->Boss = Cast<ABoss>(TaskMemory->AIController->GetPawn());
	
	if (!TaskMemory->Boss.IsValid() || !TaskMemory->AIController.IsValid()) return EBTNodeResult::Failed;
	
	TaskMemory->Boss->SetBossState(EBossState::Idle);
	TaskMemory->AccumulatedTime = 0.f;
	TaskMemory->IdleFinishTime = FMath::RandRange(IdleFinishMinInterval, IdleFinishMaxInterval);
	
	return EBTNodeResult::InProgress;
}

void UBTTask_EnhancedIdle::TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(Comp, NodeMemory, DeltaSeconds);

	FBTIdleTaskMemory* TaskMemory = (FBTIdleTaskMemory*)NodeMemory;
	if (!TaskMemory) return;
	
	if (TaskMemory->AccumulatedTime > TaskMemory->IdleFinishTime)
	{
		FinishLatentTask(Comp, EBTNodeResult::Succeeded);
	}
	
	TaskMemory->AccumulatedTime += FMath::Clamp(DeltaSeconds, 0.f, 0.1f);
}
