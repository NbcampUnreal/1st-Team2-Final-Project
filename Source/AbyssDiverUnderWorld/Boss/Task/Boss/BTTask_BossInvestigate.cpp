#include "Boss/Task/Boss/BTTask_BossInvestigate.h"
#include "AbyssDiverUnderWorld.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Boss/Boss.h"
#include "Boss/EBossState.h"
#include "Navigation/PathFollowingComponent.h"

const FName UBTTask_BossInvestigate::BossStateKey = "BossState";

UBTTask_BossInvestigate::UBTTask_BossInvestigate()
{
	NodeName = TEXT("Boss Investigate");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_BossInvestigate::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 컨트롤러 캐스팅에 실패하면 얼리 리턴
	ABossAIController* AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	// 보스 캐스팅에 실패하면 얼리 리턴
	ABoss* Boss = Cast<ABoss>(AIController->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	// 보스의 시야각을 정상으로 전환한 후 마지막으로 플레이어를 목격한 위치로 이동
	AIController->SetVisionAngle(AIController->DefaultVisionAngle);
	AIController->MoveToLocationWithRadius(Boss->LastDetectedLocation);

	// TickTask로 전이
	return EBTNodeResult::InProgress;
}

void UBTTask_BossInvestigate::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	// 캐스팅
	ABossAIController* AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	ABoss* Boss = Cast<ABoss>(AIController->GetCharacter());

	// 마지막으로 플레이어를 목격한 위치를 향해 보스 회전
	Boss->RotationToTarget(Boss->LastDetectedLocation);
	
	// 해당 위치로 이동 완료한 경우 Idle 상태로 전이
	if (AIController->GetPathFollowingComponent()->GetStatus() == EPathFollowingStatus::Idle)
	{
		Boss->SetBossState(EBossState::Idle);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
}
