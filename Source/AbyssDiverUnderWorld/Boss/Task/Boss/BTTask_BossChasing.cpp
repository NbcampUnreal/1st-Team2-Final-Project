#include "Boss/Task/Boss/BTTask_BossChasing.h"
#include "AbyssDiverUnderWorld.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Boss/Boss.h"
#include "Boss/EBossState.h"
#include "Character/UnderwaterCharacter.h"
#include "Navigation/PathFollowingComponent.h"

const FName UBTTask_BossChasing::BossStateKey = "BossState";

UBTTask_BossChasing::UBTTask_BossChasing()
{
	NodeName = TEXT("Boss Chasing");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_BossChasing::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 컨트롤러 캐스팅 실패하면 얼리 리턴
	ABossAIController* AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	// 보스 캐스팅 실패하면 얼리 리턴
	ABoss* Boss = Cast<ABoss>(AIController->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	// 보스의 시야각을 넓게 전환한 후 이동속도 증가
	AIController->SetVisionAngle(AIController->ChasingVisionAngle);
	Boss->SetMoveSpeed(MoveSpeedMultiplier);

	// TickTask로 전이
	return EBTNodeResult::InProgress;
}

void UBTTask_BossChasing::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	// 캐스팅
	ABossAIController* AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	ABoss* Boss = Cast<ABoss>(OwnerComp.GetAIOwner()->GetCharacter());
	
	// 추적하던 타겟이 사라진 경우 Investigate 상태로 전이
	if (!IsValid(Boss->GetTarget()))
	{
		Boss->SetBossState(EBossState::Investigate);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// 추적하는 타겟 방향으로 보스 회전
	Boss->RotationToTarget(Boss->GetTarget());
	AIController->MoveToActorWithRadius();
	

	// 공격 가능한 범위가 들어온 경우 **공격 상태**로 전이
	if (Boss->GetIsAttackCollisionOverlappedPlayer())
	{
		Boss->SetBossState(EBossState::Attack);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// 타겟 앞으로 도착했을 때에도 **공격 상태**로 전이
	if (AIController->GetPathFollowingComponent()->GetStatus() == EPathFollowingStatus::Idle)
	{
		Boss->SetBossState(EBossState::Attack);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// 추적 상태가 끝났다면 성공 반환
	if (AIController->GetBlackboardComponent()->GetValueAsEnum(BossStateKey) != static_cast<uint8>(EBossState::Chase))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
}
