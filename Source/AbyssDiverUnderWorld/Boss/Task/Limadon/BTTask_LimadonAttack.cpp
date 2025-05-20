#include "Boss/Task/Limadon/BTTask_LimadonAttack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Boss/Boss.h"
#include "Boss/EBossState.h"
#include "Boss/Limadon/Limadon.h"

const FName UBTTask_LimadonAttack::BossStateKey = "BossState";

UBTTask_LimadonAttack::UBTTask_LimadonAttack()
{
	NodeName = TEXT("Limadon Attack");
}

EBTNodeResult::Type UBTTask_LimadonAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 컨트롤러 캐스팅 실패하면 얼리 리턴
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!IsValid(Controller)) return EBTNodeResult::Failed;
	
	// 보스 캐스팅 실패하면 얼리 리턴
	ALimadon* Limadon = Cast<ALimadon>(OwnerComp.GetAIOwner()->GetPawn());
	if (!IsValid(Limadon)) return EBTNodeResult::Failed;
	
	// 리마돈 공격 애니메이션 출력
	Limadon->Attack();

	// 액터 활성화
	Limadon->M_SetLimadonVisible();
	
	// 결과 성공 반환
	return EBTNodeResult::InProgress;
}

void UBTTask_LimadonAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	// 공격 상태가 끝났다면 결과 반환
	if (static_cast<EBossState>(OwnerComp.GetBlackboardComponent()->GetValueAsEnum(BossStateKey)) != EBossState::Attack)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
}
