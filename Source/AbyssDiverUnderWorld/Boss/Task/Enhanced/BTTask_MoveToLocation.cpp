#include "Boss/Task/Enhanced/BTTask_MoveToLocation.h"
#include "AbyssDiverUnderWorld.h"
#include "NavigationSystem.h"
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
	bCreateNodeInstance = false;
	
	bIsInitialized = true;
	MinFinishTaskInterval = 3.f;
	MaxFinishTaskInterval = 6.f;
	DecelerationTriggeredRadius = 2000.0f;
}

EBTNodeResult::Type UBTTask_MoveToLocation::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	FBTMoveToLocationTaskMemory* TaskMemory = (FBTMoveToLocationTaskMemory*)NodeMemory;
	if (!TaskMemory) return EBTNodeResult::Failed;

	TaskMemory->AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	TaskMemory->Boss = Cast<ABoss>(TaskMemory->AIController->GetCharacter());
	
	if (!TaskMemory->Boss.IsValid() || !TaskMemory->AIController.IsValid()) return EBTNodeResult::Failed;

	// 감속을 멈추고 이동 상태로 전이하여 ABP에서 이동 상태임을 인지
	TaskMemory->Boss->SetBossState(EBossState::Move);
	
	// Task에 할당된 블랙보드 키 값을 추출
	const FName KeyName = GetSelectedBlackboardKey();
	TaskMemory->TargetLocation = TaskMemory->AIController->GetBlackboardComponent()->GetValueAsVector(KeyName);

	// 디버그용 구체 출력 (5초 동안, 반지름 50, 빨간색)
	DrawDebugSphere(GetWorld(), TaskMemory->TargetLocation, 250.0f, 12, FColor::Green, false, 3.f);

	// MoveTask를 종료할 랜덤 시간 추출
	TaskMemory->FinishTaskInterval = FMath::RandRange(MinFinishTaskInterval, MaxFinishTaskInterval);
	TaskMemory->AccumulatedTime = 0.f;

	const EPathFollowingRequestResult::Type Result = TaskMemory->AIController->MoveToLocationWithRadius(TaskMemory->TargetLocation);
	
	// AI가 NavMesh를 벗어난 상태인 경우
	if (Result == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		TaskMemory->bShouldMoveToNearestPoint = true;
	}
	
	// AI가 지형에 막힌 경우
	if (TaskMemory->bHasBeenTriggeredMoveToLocation && FVector::Dist(TaskMemory->TargetLocation, TaskMemory->CachedLocation) <= 1.f)
	{
		TaskMemory->bShouldMoveToNearestPoint = true;
	}

	// AI가 지형에 막힌 상태이거나 NavMesh를 벗어난 상태인 경우
	if (TaskMemory->bShouldMoveToNearestPoint)
	{
		LOG(TEXT("AI %s is Stuck ! Should Move AI to Nearest NavMesh"), *TaskMemory->AIController->GetName());

		return EBTNodeResult::Failed;
	}

	// 이동 요청이 성공적으로 처리된 경우
	if (Result == EPathFollowingRequestResult::RequestSuccessful)
	{
		LOG(TEXT("AI Move Request Successful"));
		return EBTNodeResult::InProgress;
	}

	// 위의 요청들이 False인 경우 예외 상황이므로 Fail 처리
	return EBTNodeResult::Failed;
}

void UBTTask_MoveToLocation::TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(Comp, NodeMemory, DeltaSeconds);

	FBTMoveToLocationTaskMemory* TaskMemory = (FBTMoveToLocationTaskMemory*)NodeMemory;
	if (!TaskMemory) return;

	TaskMemory->AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	TaskMemory->Boss = Cast<ABoss>(TaskMemory->AIController->GetCharacter());
	
	if (!TaskMemory->Boss.IsValid() || !TaskMemory->AIController.IsValid()) return;
	
	// 해당 지점에 도착한 경우 테스크 종료
	if (TaskMemory->AIController->GetPathFollowingComponent()->GetStatus() == EPathFollowingStatus::Idle)
	{
		LOG(TEXT("AI has Arrived at Target Location"));
		if (bIsInitialized)
		{
			TaskMemory->AIController->InitVariables();	
		}
		
		FinishLatentTask(Comp, EBTNodeResult::Succeeded);
		return;
	}
}

void UBTTask_MoveToLocation::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	FBTMoveToLocationTaskMemory* TaskMemory = (FBTMoveToLocationTaskMemory*)NodeMemory;
	if (!TaskMemory) return;

	TaskMemory->AIController = Cast<AEnhancedBossAIController>(OwnerComp.GetAIOwner());
	TaskMemory->Boss = Cast<ABoss>(TaskMemory->AIController->GetCharacter());
	
	if (!TaskMemory->Boss.IsValid() || !TaskMemory->AIController.IsValid()) return;

	TaskMemory->CachedLocation = TaskMemory->TargetLocation;
	TaskMemory->bHasBeenTriggeredMoveToLocation = true;
	TaskMemory->bShouldMoveToNearestPoint = false;
}


