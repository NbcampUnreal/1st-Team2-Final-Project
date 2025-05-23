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
	TargetLocation = FVector::ZeroVector;
}

EBTNodeResult::Type UBTTask_EnhancedHide::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	Boss = Cast<ABoss>(AIController->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	// 블랙보드에 할당된 키 추출
	const FName KeyName = GetSelectedBlackboardKey();
	TargetLocation = AIController->GetBlackboardComponent()->GetValueAsVector(KeyName);

	// 해당 지점으로 이동
	AIController->MoveToLocationWithRadius(TargetLocation);
	
	return EBTNodeResult::InProgress;
}

void UBTTask_EnhancedHide::TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(Comp, NodeMemory, DeltaSeconds);

	// 목표 지점으로 이동 완료 후 Success 반환
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

	AIController->InitVariables();
}
