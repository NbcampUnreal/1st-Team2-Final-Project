#include "Boss/Task/Boss/BTTask_BossAttack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Boss/Boss.h"
#include "Boss/EBossState.h"

const FName UBTTask_BossAttack::BossStateKey = "BossState";

UBTTask_BossAttack::UBTTask_BossAttack()
{
	NodeName = TEXT("Boss Attack");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_BossAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Controller 캐스팅 실패하면 얼리 리턴
	ABossAIController* AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	// 보스 캐스팅 실패하면 얼리 리턴
	ABoss* Boss = Cast<ABoss>(AIController->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	// 보스의 움직임을 멈춘 후 공격 애니메이션 재생
	AIController->StopMovement();
	Boss->Attack();

	// TickTask로 전이
	return EBTNodeResult::InProgress;
}

void UBTTask_BossAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
	
	// 공격 상태가 끝났다면 결과 반환
	if (static_cast<EBossState>(OwnerComp.GetBlackboardComponent()->GetValueAsEnum(BossStateKey)) != EBossState::Attack)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
}