#include "Monster/BT/Commons/BTTask_Damaged.h"

#include "AbyssDiverUnderWorld.h"

#include "Monster/Boss/Boss.h"
//#include "Monster/Boss/ENum/EBossState.h"
#include "Monster/EPerceptionType.h"
#include "Monster/MonsterAIController.h"

UBTTask_Damaged::UBTTask_Damaged()
{
	NodeName = "Damaged";
	bNotifyTick = true;
	bCreateNodeInstance = false;

	MaxRotationTime = 4.0f;
	MinRotationTime = 2.5f;
	RotationStartTime = 1.0f;
}

EBTNodeResult::Type UBTTask_Damaged::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	FBTDamagedTaskMemory* TaskMemory = (FBTDamagedTaskMemory*)NodeMemory;
	if (!TaskMemory) return EBTNodeResult::Failed;

	TaskMemory->AIController = Cast<AMonsterAIController>(Comp.GetAIOwner());
	TaskMemory->Boss = Cast<ABoss>(Comp.GetAIOwner()->GetCharacter());
	
	if (!TaskMemory->Boss.IsValid() || !TaskMemory->AIController.IsValid()) return EBTNodeResult::Failed;

	TaskMemory->Boss->SetMonsterState(EMonsterState::Idle);
	
	TaskMemory->AccumulatedTime = 0.0f;
	TaskMemory->FinishTaskInterval = FMath::RandRange(MinRotationTime, MaxRotationTime);
	
	return EBTNodeResult::InProgress;
}

void UBTTask_Damaged::TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(Comp, NodeMemory, DeltaSeconds);
	
	FBTDamagedTaskMemory* TaskMemory = (FBTDamagedTaskMemory*)NodeMemory;
	if (!TaskMemory) return;
	
	if (TaskMemory->AccumulatedTime > RotationStartTime)
	{
		TaskMemory->Boss->RotationToTarget(TaskMemory->Boss->GetDamagedLocation());
	}

	if (TaskMemory->AccumulatedTime > TaskMemory->FinishTaskInterval)
	{
		TaskMemory->AIController->SetBlackboardPerceptionType(EPerceptionType::None);
		FinishLatentTask(Comp, EBTNodeResult::Succeeded);
	}
	
	TaskMemory->AccumulatedTime += FMath::Clamp(DeltaSeconds, 0.f, 0.1f);
}
