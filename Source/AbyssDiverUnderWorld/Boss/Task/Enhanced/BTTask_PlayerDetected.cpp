#include "Boss/Task/Enhanced/BTTask_PlayerDetected.h"
#include "Boss/Boss.h"
#include "Character/UnderwaterCharacter.h"

UBTTask_PlayerDetected::UBTTask_PlayerDetected()
{
	NodeName = "Player Detected";
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	Boss = nullptr;
	AIController = nullptr;
	AccumulatedTime = 0;
	DetectedStateInterval = 2.f;
}

EBTNodeResult::Type UBTTask_PlayerDetected::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	Boss = Cast<ABoss>(AIController->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	AccumulatedTime = 0;
	
	return EBTNodeResult::InProgress;
}

void UBTTask_PlayerDetected::TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(Comp, NodeMemory, DeltaSeconds);

	Boss->RotationToTarget(Boss->GetTarget());

	if (AIController->GetIsDisappearPlayer())
	{
		FinishLatentTask(Comp, EBTNodeResult::Succeeded);
	}

	if (AccumulatedTime > DetectedStateInterval)
	{
		FinishLatentTask(Comp, EBTNodeResult::Succeeded);
	}
	else
	{
		AccumulatedTime += DeltaSeconds;
	}
	
}

void UBTTask_PlayerDetected::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	AIController->GetBlackboardComponent()->SetValueAsBool("bHasDetectedPlayer", true);
}
