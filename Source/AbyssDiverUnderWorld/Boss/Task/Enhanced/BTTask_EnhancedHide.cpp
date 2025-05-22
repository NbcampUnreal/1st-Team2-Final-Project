#include "Boss/Task/Enhanced/BTTask_EnhancedHide.h"
#include "AbyssDiverUnderWorld.h"
#include "Boss/Boss.h"
#include "Boss/Enum/EPerceptionType.h"

UBTTask_EnhancedHide::UBTTask_EnhancedHide()
{
	NodeName = "Enhanced Hide";
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	Boss = nullptr;
	AIController = nullptr;
	HideLocation = FVector::ZeroVector;
}

EBTNodeResult::Type UBTTask_EnhancedHide::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	LOG(TEXT("In"));
	AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	Boss = Cast<ABoss>(AIController->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	HideLocation = AIController->GetBlackboardComponent()->GetValueAsVector("HideLocation");
	AIController->MoveToLocationWithRadius(HideLocation);

	LOG(TEXT("Hide"));
	
	return EBTNodeResult::InProgress;
}

void UBTTask_EnhancedHide::TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(Comp, NodeMemory, DeltaSeconds);

	// 현재 Patrol Point로 이동 완료 후 Success 반환
	if (AIController->GetPathFollowingComponent()->GetStatus() == EPathFollowingStatus::Idle)
	{
		FinishLatentTask(Comp, EBTNodeResult::Succeeded);
		return;
	}
}

void UBTTask_EnhancedHide::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	AIController->InitBlackboardVariables();
}
