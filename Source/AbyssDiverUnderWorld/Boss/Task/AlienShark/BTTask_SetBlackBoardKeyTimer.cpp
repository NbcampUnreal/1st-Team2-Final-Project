#include "Boss/Task/AlienShark/BTTask_SetBlackBoardKeyTimer.h"
#include "AIController.h"
#include "Boss/Boss.h"
#include "Boss/EnhancedBossAIController.h"

UBTTask_SetBlackBoardKeyTimer::UBTTask_SetBlackBoardKeyTimer()
{
	NodeName = TEXT("Set BlackboardKeyTimer");
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBTTask_SetBlackBoardKeyTimer::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	FBTSetBlackBoardKeyTimerTask* TaskMemory = (FBTSetBlackBoardKeyTimerTask*)NodeMemory;
	if (!TaskMemory) return EBTNodeResult::Failed;

	TaskMemory->AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	TaskMemory->Boss = Cast<ABoss>(Comp.GetAIOwner()->GetCharacter());

	if (!TaskMemory->Boss.IsValid() || !TaskMemory->AIController.IsValid()) return EBTNodeResult::Failed;

	BlackboardKeyName = GetSelectedBlackboardKey();
	
	return EBTNodeResult::InProgress;
}

void UBTTask_SetBlackBoardKeyTimer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	FBTSetBlackBoardKeyTimerTask* TaskMemory = (FBTSetBlackBoardKeyTimerTask*)NodeMemory;
	if (!TaskMemory) return;

	TaskMemory->AccumulatedTime += FMath::Clamp(DeltaSeconds, 0.f, 0.1f);
	if (TaskMemory->AccumulatedTime > ChangeBlackboardKeyTime)
	{
		TaskMemory->AIController->GetBlackboardComponent()->SetValueAsBool(BlackboardKeyName, bBlackboardKeyValue);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

void UBTTask_SetBlackBoardKeyTimer::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	FBTSetBlackBoardKeyTimerTask* TaskMemory = (FBTSetBlackBoardKeyTimerTask*)NodeMemory;
	if (!TaskMemory) return;

	TaskMemory->AccumulatedTime = 0.f;
}
