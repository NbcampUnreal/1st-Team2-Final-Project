#include "Monster/BT/Limadon/BTTask_LimadonAttack.h"

#include "Monster/Monster.h"
//#include "Monster/Boss/Enum/EBossState.h"
#include "Monster/Boss/Limadon/Limadon.h"

#include "BehaviorTree/BlackboardComponent.h"

//const FName UBTTask_LimadonAttack::BossStateKey = "BossState";

UBTTask_LimadonAttack::UBTTask_LimadonAttack()
{
	NodeName = TEXT("Limadon Attack");
	bNotifyTick = true;
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBTTask_LimadonAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTLimadonAttackTaskMemory* TaskMemory = (FBTLimadonAttackTaskMemory*)NodeMemory;
	if (!TaskMemory) return EBTNodeResult::Failed;

	TaskMemory->AIController = Cast<AMonsterAIController>(OwnerComp.GetAIOwner());
	TaskMemory->Limadon = Cast<ALimadon>(TaskMemory->AIController->GetCharacter());
	
	if (!TaskMemory->Limadon.IsValid() || !TaskMemory->AIController.IsValid()) return EBTNodeResult::Failed;
	
	TaskMemory->Limadon->Attack();
	
	return EBTNodeResult::InProgress;
}

void UBTTask_LimadonAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	FBTLimadonAttackTaskMemory* TaskMemory = (FBTLimadonAttackTaskMemory*)NodeMemory;
	if (!TaskMemory) return;

	TaskMemory->AIController = Cast<AMonsterAIController>(OwnerComp.GetAIOwner());
	TaskMemory->Limadon = Cast<ALimadon>(TaskMemory->AIController->GetCharacter());
	
	if (!TaskMemory->Limadon.IsValid() || !TaskMemory->AIController.IsValid()) return;

	if (!TaskMemory->AIController->IsStateSame(EMonsterState::Attack))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
}
