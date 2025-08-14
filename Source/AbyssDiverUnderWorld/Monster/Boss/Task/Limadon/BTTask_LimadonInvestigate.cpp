#include "Monster/Boss/Task/Limadon/BTTask_LimadonInvestigate.h"
#include "AbyssDiverUnderWorld.h"
#include "Monster/Boss/Enum/EBossState.h"
#include "Monster/Boss/Limadon/Limadon.h"

UBTTask_LimadonInvestigate::UBTTask_LimadonInvestigate()
{
	NodeName = TEXT("Limadon Investigate");
	bNotifyTick = true;
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBTTask_LimadonInvestigate::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTLimadonInvestigateTaskMemory* TaskMemory = (FBTLimadonInvestigateTaskMemory*)NodeMemory;
	if (!TaskMemory) return EBTNodeResult::Failed;

	TaskMemory->AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	TaskMemory->Limadon = Cast<ALimadon>(TaskMemory->AIController->GetCharacter());

	if (!TaskMemory->Limadon.IsValid() || !TaskMemory->AIController.IsValid()) return EBTNodeResult::Failed;

	return EBTNodeResult::InProgress;
}

void UBTTask_LimadonInvestigate::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
	
	FBTLimadonInvestigateTaskMemory* TaskMemory = (FBTLimadonInvestigateTaskMemory*)NodeMemory;
	if (!TaskMemory) return;
	
	TaskMemory->AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	TaskMemory->Limadon = Cast<ALimadon>(TaskMemory->AIController->GetCharacter());
	
	if (!TaskMemory->Limadon.IsValid() || !TaskMemory->AIController.IsValid()) return;
	
	// 추적 가능한 상태가 아니면 얼리 리턴
	if (!TaskMemory->Limadon->GetIsInvestigate()) return;
	
	// 공격 반경에 플레이어가 들어오면 공격 상태로 전이
	if (TaskMemory->Limadon->GetIsAttackCollisionOverlappedPlayer())
	{
		TaskMemory->Limadon->SetBossState(EBossState::Attack);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
