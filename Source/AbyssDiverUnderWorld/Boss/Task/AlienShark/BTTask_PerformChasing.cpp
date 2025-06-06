#include "Boss/Task/AlienShark/BTTask_PerformChasing.h"
#include "Boss/Boss.h"
#include "Boss/EnhancedBossAIController.h"
#include "Boss/Enum/EPerceptionType.h"
#include "Character/UnderwaterCharacter.h"

const FName UBTTask_PerformChasing::bCanAttackKey = "bCanAttack";
const FName UBTTask_PerformChasing::bIsHidingKey = "bIsHiding";

UBTTask_PerformChasing::UBTTask_PerformChasing()
{
	NodeName = TEXT("Perform Chasing");
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBTTask_PerformChasing::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTPerformChasingTaskMemory* TaskMemory = (FBTPerformChasingTaskMemory*)NodeMemory;
	if (!TaskMemory) return EBTNodeResult::Failed;

	TaskMemory->AIController = Cast<AEnhancedBossAIController>(OwnerComp.GetAIOwner());
	TaskMemory->Boss = Cast<ABoss>(OwnerComp.GetAIOwner()->GetCharacter());

	if (!TaskMemory->AIController.IsValid() || !TaskMemory->Boss.IsValid()) return EBTNodeResult::Failed;

	TaskMemory->ChasingTime = FMath::RandRange(MinChasingTime, MaxChasingTime);

	return EBTNodeResult::InProgress;
}

void UBTTask_PerformChasing::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	FBTPerformChasingTaskMemory* TaskMemory = (FBTPerformChasingTaskMemory*)NodeMemory;
	if (!TaskMemory) return;

	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(TaskMemory->AIController->GetBlackboardComponent()->GetValueAsObject("TargetPlayer"));
	
	// 플레이어가 유효하지 않은 경우 ...
	if (!IsValid(Player))
	{
		LOG(TEXT("PerformChasing: Player is not valid"));
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}


	// 플레이어가 NavMesh 밖으로 벗어난 경우 ...
	if (!TaskMemory->Boss->IsLocationOnNavMesh(Player->GetActorLocation()))
	{
		LOG(TEXT("PerformChasing: Player is not on NavMesh"));
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}


	// 추적 시간이 경과한 경우 ...
	TaskMemory->AccumulatedTime += FMath::Clamp(DeltaSeconds, 0.0f, 0.1f);
	if (TaskMemory->AccumulatedTime > TaskMemory->ChasingTime)
	{
		LOG(TEXT("PerformChasing: Chasing time exceeded"));
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}

	// 공격 영역 내에 들어온 경우 ...
	if (!bHasAttacked && TaskMemory->Boss->GetIsAttackCollisionOverlappedPlayer())
	{
		LOG(TEXT("PerformChasing: Player Is Overlapped With Attack Collision"));
		bHasAttacked = true;
		TaskMemory->AIController->GetBlackboardComponent()->SetValueAsBool(bCanAttackKey, true);
	}
	
	TaskMemory->Boss->PerformChasing(DeltaSeconds);
}

void UBTTask_PerformChasing::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	FBTPerformChasingTaskMemory* TaskMemory = (FBTPerformChasingTaskMemory*)NodeMemory;
	if (!TaskMemory) return;

	TaskMemory->AccumulatedTime = 0.0f;
	TaskMemory->Boss->InitTarget();
	TaskMemory->AIController->GetBlackboardComponent()->SetValueAsBool(bIsHidingKey, true);
	bHasAttacked = false;
}
