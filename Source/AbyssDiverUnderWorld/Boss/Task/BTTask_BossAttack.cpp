#include "Boss/Task/BTTask_BossAttack.h"
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
	ABossAIController* AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	ACharacter* Character = AIController->GetCharacter();
	if (!IsValid(Character)) return EBTNodeResult::Failed;

	ABoss* Boss = Cast<ABoss>(Character);
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	Boss->MoveStop();
	Boss->Attack();

	return EBTNodeResult::InProgress;
}

void UBTTask_BossAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	ABoss* Boss = Cast<ABoss>(OwnerComp.GetAIOwner()->GetCharacter());
	if (!IsValid(Boss))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (static_cast<EBossState>(OwnerComp.GetBlackboardComponent()->GetValueAsEnum(BossStateKey)) != EBossState::Attack)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
}