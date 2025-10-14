#include "Monster/BT/Commons/BTService_CheckTargetDistance.h"

#include "AbyssDiverUnderWorld.h"

#include "Character/UnderwaterCharacter.h"
#include "Monster/Monster.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"

UBTService_CheckTargetDistance::UBTService_CheckTargetDistance()
{
	NodeName = TEXT("Check Target Distance");
	bNotifyTick = true;
	// Tick Interval (Default : 0.5s)
	Interval = 0.5f;
	RandomDeviation = 0.1f;
}

void UBTService_CheckTargetDistance::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSecond)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSecond);

	AMonster* Monster = Cast<AMonster>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Monster) return;

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AUnderwaterCharacter* Target = Cast<AUnderwaterCharacter>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!Target) return;

	if (Target->IsGroggy() || Target->IsDeath())
	{
		LOGV(Log, TEXT("%s Is Groggy Or Dead, Remove Detection"), *Target->GetName());
		Monster->RemoveDetection(Target);
		return;
	}

	const float Distance = FVector::Dist(Monster->GetActorLocation(), Target->GetActorLocation());
	const bool bInMeleeRange = Distance <= MeleeRange;
	const bool bInRangedRange = Distance <= RangedRange;

	Blackboard->SetValueAsBool(InMeleeRangeKey.SelectedKeyName, bInMeleeRange);
	Blackboard->SetValueAsBool(InRangedRangeKey.SelectedKeyName, bInRangedRange);
}
