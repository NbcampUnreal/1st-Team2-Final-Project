#include "Monster/Boss/Task/Enhanced/BTTask_MoveToLocation.h"
#include "AbyssDiverUnderWorld.h"
#include "NavigationSystem.h"
#include "Monster/Boss/Boss.h"
#include "Monster/Boss/ENum/EBossState.h"
#include "Monster/Boss/Enum/EPerceptionType.h"

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
	MinFinishTaskInterval = 15.f;
}

EBTNodeResult::Type UBTTask_MoveToLocation::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	FBTMoveToLocationTaskMemory* TaskMemory = (FBTMoveToLocationTaskMemory*)NodeMemory;
	if (!TaskMemory) return EBTNodeResult::Failed;

	TaskMemory->AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	TaskMemory->Boss = Cast<ABoss>(Comp.GetAIOwner()->GetCharacter());
	
	if (!TaskMemory->Boss.IsValid() || !TaskMemory->AIController.IsValid()) return EBTNodeResult::Failed;

	// 이동 상태로 전이하여 ABP에서 이동 상태임을 인지
	TaskMemory->Boss->SetBossState(EBossState::Move);
	
	// Task에 할당된 블랙보드 키 값을 추출
	const FName KeyName = GetSelectedBlackboardKey();
	TaskMemory->TargetLocation = TaskMemory->AIController->GetBlackboardComponent()->GetValueAsVector(KeyName);
	
	// MoveTask를 종료할 랜덤 시간 추출
	TaskMemory->AccumulatedTime = 0.f;

	Result = TaskMemory->AIController->MoveToLocationWithRadius(TaskMemory->TargetLocation);

	return EBTNodeResult::InProgress;
}

void UBTTask_MoveToLocation::TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(Comp, NodeMemory, DeltaSeconds);

	FBTMoveToLocationTaskMemory* TaskMemory = (FBTMoveToLocationTaskMemory*)NodeMemory;
	if (!TaskMemory) return;

	
	// 만약 MoveToLocation Task 노드의 점유 시간이 MinFinishTaskInterval 이상이 된다면
	// 현재 Task에 이상이 있다는 의미이므로 주변 NavMesh 지점으로 이동한다.
	TaskMemory->AccumulatedTime += FMath::Clamp(DeltaSeconds, 0.f, 0.1f);
	if (TaskMemory->AccumulatedTime >= MinFinishTaskInterval)
	{
		LOG(TEXT("AI Name %s : MoveToLocation Task has been running for too long, Teleporting to Cached Location"), *TaskMemory->Boss->GetName());
		TeleportToNearestNavMeshLocation(TaskMemory);
	}


	
	// MoveTo의 결과가 Fail인 것은 NavMesh를 벗어났다는 의미이다.
	// 주변에 이동 가능한 NavMesh 지점으로 이동한 후 다시 Patrol Point를 할당 받는다.
	if (Result == EPathFollowingRequestResult::Failed)
	{
		LOG(TEXT("AI Name %s : MoveToLocation Failed, Trying to Move to Cached Location"), *TaskMemory->Boss->GetName());
		TeleportToNearestNavMeshLocation(TaskMemory);
		return;
	}


	
	// 이전에 이동한 지점과 거리가 0.0001f 만큼 차이가 난다는 것은 벽에 끼었다는 것이다.
	// 따라서 주변의 이동 가능한 NavMesh 지점으로 이동한다.
	if (FVector::Dist(TaskMemory->TargetLocation, TaskMemory->CachedLocation) < KINDA_SMALL_NUMBER)
	{
		LOG(TEXT("AI Stuck, Trying to Move to Cached Location"));
		TeleportToNearestNavMeshLocation(TaskMemory);
		return;
	}


	
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
}

void UBTTask_MoveToLocation::TeleportToNearestNavMeshLocation(FBTMoveToLocationTaskMemory* TaskMemory)
{
	// 주변의 가장 가까운 NavMesh 점을 찾아서 로그 출력
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(TaskMemory->Boss->GetWorld());
	if (NavSys)
	{
		FNavLocation NearestNavLocation;
		const FVector CurrentLocation = TaskMemory->Boss->GetActorLocation();
		
		bool bFound = NavSys->ProjectPointToNavigation(
			CurrentLocation,                   // 검색 기준 위치
			NearestNavLocation,               // 결과
			FVector(100.f, 100.f, 100.f)      // 탐색 반경
		);

		if (bFound)
		{
			LOG(TEXT("Nearest NavMesh Location: %s"), *NearestNavLocation.Location.ToString());
		}
		else
		{
			LOG(TEXT("Failed to find nearest NavMesh location."));
		}

		TaskMemory->Boss->SetActorLocation(NearestNavLocation.Location);
	}

	TaskMemory->CachedLocation = FVector::ZeroVector;
	TaskMemory->AccumulatedTime = 0.f;
	TaskMemory->TargetLocation = TaskMemory->Boss->GetNextPatrolPoint();
}


