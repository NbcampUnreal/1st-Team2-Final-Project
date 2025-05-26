#include "Boss/Task/Enhanced/BTTask_Damaged.h"

#include "AbyssDiverUnderWorld.h"
#include "Boss/Boss.h"
#include "Boss/ENum/EBossState.h"
#include "Boss/Enum/EPerceptionType.h"

UBTTask_Damaged::UBTTask_Damaged()
{
	NodeName = "Damaged";
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	Boss = nullptr;
	AIController = nullptr;

	MaxRotationTime = 4.0f;
	MinRotationTime = 2.5f;
	RotationStartTime = 1.0f;
	AccumulatedTime = 0.0f;
	TimeCriteria = 0.0f;
}

EBTNodeResult::Type UBTTask_Damaged::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	Boss = Cast<ABoss>(AIController->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	Boss->SetBossState(EBossState::Idle);
	Boss->SetDecelerate(true);
	
	AccumulatedTime = 0.0f;
	TimeCriteria = FMath::RandRange(MinRotationTime, MaxRotationTime);
	
	return EBTNodeResult::InProgress;
}

void UBTTask_Damaged::TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(Comp, NodeMemory, DeltaSeconds);
	
	if (AccumulatedTime > RotationStartTime)
	{
		Boss->RotationToTarget(Boss->GetDamagedLocation());
	}

	if (AccumulatedTime > TimeCriteria)
	{
		AIController->SetBlackboardPerceptionType(EPerceptionType::None);
		FinishLatentTask(Comp, EBTNodeResult::Succeeded);
	}
	
	AccumulatedTime += FMath::Clamp(DeltaSeconds, 0.f, 0.1f);
}

void UBTTask_Damaged::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	//AIController->InitBlackboardVariables();
}
