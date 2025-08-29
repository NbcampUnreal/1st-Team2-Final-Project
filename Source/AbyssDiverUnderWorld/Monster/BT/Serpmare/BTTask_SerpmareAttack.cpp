#include "Monster/BT/Serpmare/BTTask_SerpmareAttack.h"

#include "Monster/Boss/Enum/EBossState.h"
#include "Monster/Boss/Serpmare/Serpmare.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTTask_SerpmareAttack::UBTTask_SerpmareAttack()
{
	NodeName = TEXT("Serpmare Attack");
	bNotifyTick = true;
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBTTask_SerpmareAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTSerpmareAttackTaskMemory* TaskMemory = (FBTSerpmareAttackTaskMemory*)NodeMemory;
	if (!TaskMemory) return EBTNodeResult::Failed;

	TaskMemory->AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	TaskMemory->Serpmare = Cast<ASerpmare>(OwnerComp.GetAIOwner()->GetCharacter());

	if (!TaskMemory->AIController.IsValid() || !TaskMemory->Serpmare.IsValid()) return EBTNodeResult::Failed;

	TaskMemory->Serpmare->Attack();
	
	return EBTNodeResult::InProgress;
}

void UBTTask_SerpmareAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	FBTSerpmareAttackTaskMemory* TaskMemory = (FBTSerpmareAttackTaskMemory*)NodeMemory;
	if (!TaskMemory) return;

	if (!TaskMemory->AIController->IsStateSame(EBossState::Attack))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}