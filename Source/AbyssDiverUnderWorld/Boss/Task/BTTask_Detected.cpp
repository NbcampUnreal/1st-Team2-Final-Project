#include "Boss/Task/BTTask_Detected.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Boss/Boss.h"
#include "Boss/EBossState.h"

const FName UBTTask_Detected::BossStateKey = "BossState";

UBTTask_Detected::UBTTask_Detected()
{
	NodeName = TEXT("Detected Target");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_Detected::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ABossAIController* AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	ACharacter* Character = AIController->GetCharacter();
	if (!IsValid(Character)) return EBTNodeResult::Failed;

	ABoss* Boss = Cast<ABoss>(Character);
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	Boss->MoveStop();
	Boss->M_PlayAnimation(Boss->DetectedAnimation);

	return EBTNodeResult::InProgress;
}

void UBTTask_Detected::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	ABoss* Boss = Cast<ABoss>(OwnerComp.GetAIOwner()->GetCharacter());
	if (!IsValid(Boss))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (static_cast<EBossState>(OwnerComp.GetBlackboardComponent()->GetValueAsEnum(BossStateKey)) != EBossState::Detected)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	Boss->RotationToTarget();
}
