#include "Monster/Boss/Task/Serpmare/BTTask_GetOverlappedPlayer.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Monster/Boss/Boss.h"
#include "Character/UnderwaterCharacter.h"
#include "Components/CapsuleComponent.h"

UBTTask_GetOverlappedPlayer::UBTTask_GetOverlappedPlayer()
{
	NodeName = TEXT("Get Overlapped Player");
	bNotifyTick = false;
	bNotifyTaskFinished = false;
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBTTask_GetOverlappedPlayer::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	ABossAIController* AIController = Cast<ABossAIController>(Comp.GetAIOwner());
	ABoss* Boss = Cast<ABoss>(Comp.GetAIOwner()->GetCharacter());
	
	if (!IsValid(AIController) || !IsValid(Boss)) return EBTNodeResult::Failed;

	const FName BlackboardKeyName = GetSelectedBlackboardKey();

	TArray<AActor*> OverlappedActors;
	Boss->AttackCollision->GetOverlappingActors(OverlappedActors);

	for (AActor* OverlappedActor : OverlappedActors)
	{
		AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(OverlappedActor);
		if (IsValid(Player))
		{
			AIController->GetBlackboardComponent()->SetValueAsObject(BlackboardKeyName, Player);
			break;
		}
	}
	
	return EBTNodeResult::Succeeded;
}
