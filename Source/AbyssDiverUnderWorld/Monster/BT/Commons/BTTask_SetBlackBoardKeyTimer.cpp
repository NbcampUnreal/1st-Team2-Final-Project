#include "Monster/BT/Commons/BTTask_SetBlackBoardKeyTimer.h"

#include "Monster/Monster.h"
#include "Monster/MonsterAIController.h"

#include "AIController.h"

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

	TaskMemory->AIController = Cast<AMonsterAIController>(Comp.GetAIOwner());
	TaskMemory->Monster = Cast<AMonster>(Comp.GetAIOwner()->GetCharacter());

	if (!TaskMemory->Monster.IsValid() || !TaskMemory->AIController.IsValid()) return EBTNodeResult::Failed;

	BlackboardKeyName = GetSelectedBlackboardKey();
	
	return EBTNodeResult::InProgress;
}

void UBTTask_SetBlackBoardKeyTimer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	FBTSetBlackBoardKeyTimerTask* TaskMemory = (FBTSetBlackBoardKeyTimerTask*)NodeMemory;
	if (!TaskMemory) return;

	TaskMemory->AccumulatedTime += DeltaSeconds;
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
