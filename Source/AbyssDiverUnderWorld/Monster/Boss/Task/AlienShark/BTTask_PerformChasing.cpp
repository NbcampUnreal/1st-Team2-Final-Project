#include "Monster/Boss/Task/AlienShark/BTTask_PerformChasing.h"
#include "Monster/Boss/Boss.h"
#include "Monster/Boss/EnhancedBossAIController.h"
#include "Monster/Boss/Enum/EPerceptionType.h"
#include "Character/UnderwaterCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"

const FName UBTTask_PerformChasing::bCanAttackKey = "bCanAttack";
const FName UBTTask_PerformChasing::bIsHidingKey = "bIsHiding";
const FName UBTTask_PerformChasing::bIsPlayerHiddenKey = "bIsPlayerHidden";
const FName UBTTask_PerformChasing::TargetPlayerKey = "TargetPlayer";

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
		TaskMemory->AIController->GetBlackboardComponent()->SetValueAsBool(bIsHidingKey, true);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
	

	// 플레이어가 NavMesh 밖으로 벗어난 경우 ...
	//if (!TaskMemory->Boss->IsLocationOnNavMesh(Player->GetActorLocation()))
	//{
	//	LOG(TEXT("PerformChasing: Player is not on NavMesh"));
	//	TaskMemory->AIController->GetBlackboardComponent()->SetValueAsBool(bIsHidingKey, true);
	//	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	//	return;
	//}


	// 추적 시간이 경과한 경우 ...
	TaskMemory->AccumulatedTime += FMath::Clamp(DeltaSeconds, 0.0f, 0.1f);
	if (TaskMemory->AccumulatedTime > TaskMemory->ChasingTime)
	{
		LOG(TEXT("PerformChasing: Chasing time exceeded"));
		TaskMemory->AIController->GetBlackboardComponent()->SetValueAsBool(bIsHidingKey, true);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// 공격 영역 내에 들어온 경우 ...
	if (!bHasAttacked && TaskMemory->Boss->GetIsAttackCollisionOverlappedPlayer())
	{
		LOG(TEXT("PerformChasing: Player Is Overlapped With Attack Collision"));
		bHasAttacked = true;
		TaskMemory->AIController->GetBlackboardComponent()->SetValueAsBool(bCanAttackKey, true);
	}


	// 추적 중인 플레이어가 시야에서 벗어났는지 확인한다.
	// 플레이어가 시야에서 벗어난 상태에서 해초에 숨었다면
	// 플레이어 주변의 가장 가까운 점으로 이동한다.
	FActorPerceptionBlueprintInfo PerceptionInfo;
	const bool bIsPerceptionSuccess = TaskMemory->AIController->PerceptionComponent->GetActorsPerception(Player, PerceptionInfo);
	if (bIsPerceptionSuccess)
	{
		bool bIsPlayerInSight = false;
		
		for (const FAIStimulus& Stimulus : PerceptionInfo.LastSensedStimuli)
		{
			if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>() && 
				Stimulus.WasSuccessfullySensed())
			{
				bIsPlayerInSight = true;
				break;
			}
		}

		if (!bIsPlayerInSight)
		{
			if (Player->IsHideInSeaweed())
			{
				TaskMemory->AIController->GetBlackboardComponent()->SetValueAsBool(bIsPlayerHiddenKey, true);
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
				return;
			}
		}
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
	bHasAttacked = false;
}
