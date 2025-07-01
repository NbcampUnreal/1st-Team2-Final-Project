// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BT/BTService_SetMonsterState.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Monster/EMonsterState.h"
#include "Monster/Monster.h"
#include "Character/UnderwaterCharacter.h"
#include "AIController.h"

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
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return;
	APawn* AIPawn = AIController->GetPawn();
	if (!AIPawn) return;
	AMonster* Monster = Cast<AMonster>(AIPawn);
	if (!Monster) return;

	const uint8 MonsterState = Blackboard->GetValueAsEnum(MonsterStateKey.SelectedKeyName);

	switch (static_cast<EMonsterState>(MonsterState))
	{
	case EMonsterState::Chase:
	{
		if (Monster->TargetActor == nullptr)
		{
			Monster->SetMonsterState(EMonsterState::Patrol);
		}
		else
		{
			AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(Monster->TargetActor);
			if (Player)
			{
				if (Player->GetCharacterState() == ECharacterState::Death)
				{
					Monster->SetMonsterState(EMonsterState::Patrol);
				}
			}
		}
		break;
	}
	default:
		break;
	}
}
