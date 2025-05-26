#include "Boss/Task/Enhanced/BTTask_MoveToLocation.h"
#include "AbyssDiverUnderWorld.h"
#include "Boss/Boss.h"
#include "Boss/ENum/EBossState.h"

// ----- 기능 -----
// 1. Task에 할당한 Blackboard Key를 FName으로 가져온다.
// 2. FName으로 Blackboard에 접근하여 FVector로 가져온다.
// 3. AIController의 "MoveToLocationAcceptanceRadius"을 만족할 때까지 전방벡터를 향해 이동한다.
// 4. 목표 지점에 **도달했다면** AIController의 기본적인 변수들을 초기화한다.
// 5. Idle 상태로 전이한다.

// ----- 사용처 -----
// 플레이어의 피를 감지한 이후 해당 지점으로 이동할 때
// Idle 상태 이후 Move 상태에서 이동할 때
// Attack 상태 이후 Hide 상태에서 이동할 때

UBTTask_MoveToLocation::UBTTask_MoveToLocation()
{
	NodeName = "Move To Location";
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	Boss = nullptr;
	AIController = nullptr;
	TargetLocation = FVector::ZeroVector;
}

EBTNodeResult::Type UBTTask_MoveToLocation::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	Boss = Cast<ABoss>(AIController->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	Boss->SetBossState(EBossState::Move);
	
	const FName KeyName = GetSelectedBlackboardKey();
	TargetLocation = AIController->GetBlackboardComponent()->GetValueAsVector(KeyName);

	AIController->MoveToLocationWithRadius(TargetLocation);
	Boss->SetMoveSpeed(MoveSpeedMultiplier);
	
	return EBTNodeResult::InProgress;
}

void UBTTask_MoveToLocation::TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(Comp, NodeMemory, DeltaSeconds);
	
	Boss->MoveForward(DeltaSeconds);
	
	if (AIController->GetPathFollowingComponent()->GetStatus() == EPathFollowingStatus::Idle)
	{
		AIController->InitVariables();	
		FinishLatentTask(Comp, EBTNodeResult::Succeeded);
		return;
	}
}

void UBTTask_MoveToLocation::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	Boss->InitCurrentMoveSpeed();
}
