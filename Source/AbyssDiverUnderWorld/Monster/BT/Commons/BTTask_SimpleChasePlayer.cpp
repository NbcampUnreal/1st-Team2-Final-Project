#include "Monster/BT/Commons/BTTask_SimpleChasePlayer.h"

#include "AbyssDiverUnderWorld.h"
#include "Container/BlackboardKeys.h"

#include "Monster/Monster.h"
#include "Character/UnderwaterCharacter.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_SimpleChasePlayer::UBTTask_SimpleChasePlayer()
{
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_SimpleChasePlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AMonster* Monster = Cast<AMonster>(OwnerComp.GetAIOwner()->GetPawn());
	if (Monster == nullptr)
	{
		LOGV(Error, TEXT("UBTTask_SimpleChasePlayer::ExecuteTask : Monster is nullptr"));
		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_SimpleChasePlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	AMonster* Monster = Cast<AMonster>(AIController->GetPawn());
	if (Monster == nullptr)
	{
		LOGV(Error, TEXT("UBTTask_SimpleChasePlayer::TickTask : Monster is nullptr"));
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(AIController->GetBlackboardComponent()->GetValueAsObject(BlackboardKeys::TargetPlayerKey));
	if (Player == nullptr)
	{
		LOGV(Error, TEXT("Player IS Not Valid"));
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (Player->IsHideInSeaweed())
	{
		//AIController->GetBlackboardComponent()->SetValueAsBool("bIsPlayerHidden", true);
		Monster->RemoveDetection(Player);
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	Monster->PerformChasing(DeltaSeconds);
}
