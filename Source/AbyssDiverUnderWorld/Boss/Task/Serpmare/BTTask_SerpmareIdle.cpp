#include "Boss/Task/Serpmare/BTTask_SerpmareIdle.h"

#include "Boss/EBossState.h"
#include "Boss/Serpmare/Serpmare.h"

UBTTask_SerpmareIdle::UBTTask_SerpmareIdle()
{
	NodeName = TEXT("Serpmare Idle");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_SerpmareIdle::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 캐스팅 실패 시 얼리 리턴
	ABossAIController* AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;
	
	ASerpmare* Serpmare = Cast<ASerpmare>(AIController->GetCharacter());
	if (!Serpmare) return EBTNodeResult::Failed;
	
	return EBTNodeResult::InProgress;
}

void UBTTask_SerpmareIdle::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	// 캐스팅
	ASerpmare* Serpmare = Cast<ASerpmare>(OwnerComp.GetAIOwner()->GetCharacter());
	if (!IsValid(Serpmare)) return;

	// 공격 범위 내에 플레이어가 들어온 경우
	if (Serpmare->GetIsAttackCollisionOverlappedPlayer())
	{
		Serpmare->SetBossState(EBossState::Detected);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
