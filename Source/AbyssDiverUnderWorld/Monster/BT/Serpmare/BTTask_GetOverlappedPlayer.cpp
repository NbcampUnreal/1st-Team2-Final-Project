#include "Monster/BT/Serpmare/BTTask_GetOverlappedPlayer.h"

#include "Monster/Monster.h"
#include "Character/UnderwaterCharacter.h"

#include "Components/CapsuleComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h" 

UBTTask_GetOverlappedPlayer::UBTTask_GetOverlappedPlayer()
{
	NodeName = TEXT("Get Overlapped Player");
	bNotifyTick = false;
	bNotifyTaskFinished = false;
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBTTask_GetOverlappedPlayer::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	AAIController* AIController = Cast<AAIController>(Comp.GetAIOwner());
	AMonster* Monster = Cast<AMonster>(Comp.GetAIOwner()->GetCharacter());
	
	if (!IsValid(AIController) || !IsValid(Monster)) return EBTNodeResult::Failed;

	const FName BlackboardKeyName = GetSelectedBlackboardKey();

	TArray<AActor*> OverlappedActors;
	Monster->AttackCollision->GetOverlappingActors(OverlappedActors);

	for (AActor* OverlappedActor : OverlappedActors)
	{
		AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(OverlappedActor);
		if (IsValid(Player))
		{
			Monster->SetTarget(Player);
			break;
		}
	}
	
	return EBTNodeResult::Succeeded;
}
