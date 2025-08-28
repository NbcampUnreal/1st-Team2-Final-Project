#include "Monster/Boss/Task/AlienShark/BTTask_PlayAnimMontage.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Monster/Boss/Boss.h"

UBTTask_PlayAnimMontage::UBTTask_PlayAnimMontage()
{
	NodeName = TEXT("Play Anim Montage");
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBTTask_PlayAnimMontage::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ABoss* Boss = Cast<ABoss>(OwnerComp.GetAIOwner()->GetCharacter());
	if (!IsValid(Boss) || !IsValid(AnimMontage)) return EBTNodeResult::Failed;

	AccumulatedTime = 0.0f;
	Boss->M_PlayMontage(AnimMontage);
	
	return EBTNodeResult::InProgress;
}

void UBTTask_PlayAnimMontage::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	AccumulatedTime += FMath::Clamp(DeltaSeconds, 0.0f, 0.1f);
	if (AccumulatedTime >= AnimMontageLength)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
}

void UBTTask_PlayAnimMontage::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
                                             EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	OwnerComp.GetBlackboardComponent()->SetValueAsBool(*BlackboardKeyInfo.KeyName, BlackboardKeyInfo.bIsActive);
}
