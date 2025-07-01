#include "Boss/Task/Enhanced/BTTask_PlayerDetected.h"
#include "AbyssDiverUnderWorld.h"
#include "Boss/Boss.h"
#include "Boss/ENum/EBossState.h"
#include "Character/UnderwaterCharacter.h"

// ----- 기능 -----
// 1. AI의 이동을 멈춘다.
// 2. 보스가 현재 추적 중인 타겟을 향해 회전한다.
// 3-1. 타겟이 시야에서 사라진 경우 Task를 종료한다.
// 3-2. 지정한 시간만큼 경과한 경우 Task를 종료한다.
// 4. Task를 종료할 때 Blackboard에 플레이어를 감지 완료 했음을 알린다.

// ----- 사용처 -----
// AI의 시야각에 플레이어가 들어온 경우

UBTTask_PlayerDetected::UBTTask_PlayerDetected()
{
	NodeName = "Player Detected";
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	bCreateNodeInstance = false;

	DetectedStateInterval = 2.f;
}

EBTNodeResult::Type UBTTask_PlayerDetected::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	FBTPlayerDetectedTaskMemory* TaskMemory = (FBTPlayerDetectedTaskMemory*)NodeMemory;
	if (!TaskMemory) return EBTNodeResult::Failed;

	TaskMemory->AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	TaskMemory->Boss = Cast<ABoss>(Comp.GetAIOwner()->GetCharacter());
	
	if (!TaskMemory->Boss.IsValid() || !TaskMemory->AIController.IsValid()) return EBTNodeResult::Failed;
	
	TaskMemory->Boss->SetBossState(EBossState::Idle);
	
	TaskMemory->AccumulatedTime = 0.f;
	
	return EBTNodeResult::InProgress;
}

void UBTTask_PlayerDetected::TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(Comp, NodeMemory, DeltaSeconds);
	
	FBTPlayerDetectedTaskMemory* TaskMemory = (FBTPlayerDetectedTaskMemory*)NodeMemory;
	if (!TaskMemory) return;
	
	TaskMemory->Boss->RotationToTarget(TaskMemory->Boss->GetTarget());

	if (TaskMemory->AIController->GetIsDisappearPlayer())
	{
		FinishLatentTask(Comp, EBTNodeResult::Succeeded);
	}

	if (TaskMemory->AccumulatedTime > DetectedStateInterval)
	{
		FinishLatentTask(Comp, EBTNodeResult::Succeeded);
	}

	TaskMemory->AccumulatedTime += FMath::Clamp(DeltaSeconds, 0.f, 0.1f);
}

void UBTTask_PlayerDetected::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	FBTPlayerDetectedTaskMemory* TaskMemory = (FBTPlayerDetectedTaskMemory*)NodeMemory;
	if (!TaskMemory) return;

	TaskMemory->AIController = Cast<AEnhancedBossAIController>(OwnerComp.GetAIOwner());
	TaskMemory->Boss = Cast<ABoss>(TaskMemory->AIController->GetCharacter());
	
	if (!TaskMemory->Boss.IsValid() || !TaskMemory->AIController.IsValid()) return;
	
	TaskMemory->AIController->GetBlackboardComponent()->SetValueAsBool("bHasDetectedPlayer", true);
}
