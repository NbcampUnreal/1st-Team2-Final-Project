#include "Boss/Task/BTTask_BossIdle.h"
#include "AIController.h"
#include "Boss/Boss.h"

UBTTask_BossIdle::UBTTask_BossIdle()
{
	NodeName = TEXT("Boss Idle");
}

EBTNodeResult::Type UBTTask_BossIdle::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 컨트롤러 캐스팅에 실패하면 얼리 리턴
	ABossAIController* AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	// 보스 캐스팅에 실패하면 얼리 리턴
	ABoss* Boss = Cast<ABoss>(AIController->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	// 보스의 움직임 정지
	AIController->StopMovement();

	// 성공 결과 반환
	return EBTNodeResult::Succeeded;
}
