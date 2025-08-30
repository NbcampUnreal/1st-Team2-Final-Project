// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BT/Commons/BTService_CheckTargetDistance.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"

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

	APawn* AIPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (!AIPawn) return;

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!Target) return;

	const float Distance = FVector::Dist(AIPawn->GetActorLocation(), Target->GetActorLocation());
	const bool bInMeleeRange = Distance <= MeleeRange;
	const bool bInRangedRange = Distance <= RangedRange;

	Blackboard->SetValueAsBool(InMeleeRangeKey.SelectedKeyName, bInMeleeRange);
	Blackboard->SetValueAsBool(InRangedRangeKey.SelectedKeyName, bInRangedRange);
}
