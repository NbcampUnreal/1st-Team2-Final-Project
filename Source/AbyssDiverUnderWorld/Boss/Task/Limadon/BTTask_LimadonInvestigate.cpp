#include "Boss/Task/Limadon/BTTask_LimadonInvestigate.h"
#include "AbyssDiverUnderWorld.h"
#include "Boss/Enum/EBossState.h"
#include "Boss/Limadon/Limadon.h"

UBTTask_LimadonInvestigate::UBTTask_LimadonInvestigate()
{
	NodeName = TEXT("Limadon Investigate");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_LimadonInvestigate::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 캐스팅에 실패하면 얼리 리턴
	ALimadon* Limadon = Cast<ALimadon>(OwnerComp.GetAIOwner()->GetPawn());
	if (!IsValid(Limadon)) return EBTNodeResult::Failed;

	// TickTask로 전이
	return EBTNodeResult::InProgress;
}

void UBTTask_LimadonInvestigate::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	// 캐스팅에 실패하면 얼리 리턴
	ALimadon* Limadon = Cast<ALimadon>(OwnerComp.GetAIOwner()->GetPawn());
	if (!IsValid(Limadon)) return;

	// 추적 가능한 상태가 아니면 얼리 리턴
	if (!Limadon->GetIsInvestigate()) return;

	// 공격 반경에 플레이어가 들어오면 공격 상태로 전이
	if (Limadon->GetIsAttackCollisionOverlappedPlayer())
	{
		Limadon->SetBossState(EBossState::Attack);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
