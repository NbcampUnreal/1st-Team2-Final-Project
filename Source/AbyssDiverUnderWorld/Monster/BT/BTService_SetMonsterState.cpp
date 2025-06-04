// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BT/BTService_SetMonsterState.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Monster/EMonsterState.h"

UBTService_SetMonsterState::UBTService_SetMonsterState()
{
	NodeName = TEXT("Set AI State");
	bNotifyBecomeRelevant = true;
	bNotifyTick = true;
	Interval = 0.3f;
	RandomDeviation = 0.f;
}

void UBTService_SetMonsterState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard) return;

	const uint8 MonsterState = Blackboard->GetValueAsEnum(MonsterStateKey.SelectedKeyName);

	switch (static_cast<EMonsterState>(MonsterState))
	{
	case EMonsterState::Chase:
	{
		AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
		if (!IsValid(Target))
		{
			Blackboard->ClearValue(TargetActorKey.SelectedKeyName);
			Blackboard->SetValueAsEnum(MonsterStateKey.SelectedKeyName, static_cast<uint8>(EMonsterState::Patrol));
		}
		break;
	}
	case EMonsterState::Investigate:
	{
		const FVector InvestigateLocation = Blackboard->GetValueAsVector(InvestigateLocationKey.SelectedKeyName);
		if (InvestigateLocation.IsZero())
		{
			Blackboard->SetValueAsEnum(MonsterStateKey.SelectedKeyName, static_cast<uint8>(EMonsterState::Patrol));
		}
		break;
	}
	default:
		break;
	}
}
