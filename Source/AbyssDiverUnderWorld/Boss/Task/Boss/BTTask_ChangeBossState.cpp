#include "Boss/Task/Boss/BTTask_ChangeBossState.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Boss/Boss.h"
#include "Boss/BossAIController.h"
#include "Boss/Enum/EBossState.h"

const FName UBTTask_ChangeBossState::BossStateKey = "BossState";

UBTTask_ChangeBossState::UBTTask_ChangeBossState()
{
	NodeName = TEXT("ChangeBossState");
	BossState = EBossState::Idle;
}

EBTNodeResult::Type UBTTask_ChangeBossState::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 컨트롤러 캐스팅 실패 시 얼리 리턴
	ABossAIController* AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	// 보스 캐스팅에 실패하면 얼리 리턴
	ABoss* Boss = Cast<ABoss>(AIController->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	// 지정한 상태로 보스의 상태 전이
	Boss->SetBossState(BossState);

	// 결과 성공 반환
	return EBTNodeResult::Succeeded;
}
