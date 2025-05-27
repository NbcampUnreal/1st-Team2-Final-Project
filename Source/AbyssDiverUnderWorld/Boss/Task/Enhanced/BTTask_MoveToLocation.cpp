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
	Boss = nullptr;
	AIController = nullptr;
	bIsInitialized = true;
	TargetLocation = FVector::ZeroVector;
	bHasBeenTriggeredMoveToLocation = false;
	bShouldMoveToNearestPoint = false;
	AccumulatedTime = 0.f;
	FinishTaskInterval = 0.f;
	MinFinishTaskInterval = 3.f;
	MaxFinishTaskInterval = 6.f;
	CachedLocation = FVector::ZeroVector;
	DecelerationTriggeredRadius = 2000.0f;
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

	FinishTaskInterval = FMath::RandRange(MinFinishTaskInterval, MaxFinishTaskInterval);

	// AI가 지형에 막힌 경우
	if (bHasBeenTriggeredMoveToLocation && FVector::Dist(TargetLocation, CachedLocation) <= 1.f)
	{
		bShouldMoveToNearestPoint = true;
	}
	
	// 디버그용 구체 출력 (5초 동안, 반지름 50, 빨간색)
	//DrawDebugSphere(GetWorld(), TargetLocation, 250.0f, 12, FColor::Red, false, 5.0f);
	
	Boss->SetDecelerate(false);
	AccumulatedTime = 0.f;

	EPathFollowingRequestResult::Type Result = AIController->MoveToLocationWithRadius(TargetLocation);

	// AI가 NavMesh를 벗어난 상태인 경우
	if (Result == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		bShouldMoveToNearestPoint = true;
	}

	// AI가 지형에 막힌 상태이거나 NavMesh를 벗어난 상태인 경우
	if (bShouldMoveToNearestPoint)
	{
		LOG(TEXT("AI is Stuck ! Should Move AI to Nearest NavMesh"));

		return EBTNodeResult::Failed;
	}

	// 이동 요청이 성공적으로 처리된 경우
	if (Result == EPathFollowingRequestResult::RequestSuccessful)
	{
		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Failed;
}

void UBTTask_MoveToLocation::TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(Comp, NodeMemory, DeltaSeconds);
	
	const float Distance = FVector::Dist(Boss->GetActorLocation(), TargetLocation);
	if (Distance < DecelerationTriggeredRadius)
	{
		Boss->SetDecelerate(true);
		
		if (AccumulatedTime > FinishTaskInterval)
		{
			if (bIsInitialized)
			{
				AIController->InitVariables();	
			}
			AccumulatedTime = 0.f;
			FinishLatentTask(Comp, EBTNodeResult::Succeeded);
			return;
		}

		AccumulatedTime += FMath::Clamp(DeltaSeconds, 0.f, 1.f);
	}
	
	if (AIController->GetPathFollowingComponent()->GetStatus() == EPathFollowingStatus::Idle)
	{
		if (bIsInitialized)
		{
			AIController->InitVariables();	
		}
		
		FinishLatentTask(Comp, EBTNodeResult::Succeeded);
		return;
	}
}

void UBTTask_MoveToLocation::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	CachedLocation = TargetLocation;
	bHasBeenTriggeredMoveToLocation = true;
	bShouldMoveToNearestPoint = false;
}


