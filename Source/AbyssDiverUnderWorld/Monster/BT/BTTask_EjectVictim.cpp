// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BT/BTTask_EjectVictim.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Monster/HorrorCreature/HorrorCreature.h"

UBTTask_EjectVictim::UBTTask_EjectVictim()
{
	NodeName = "Eject Victim";
	bCreateNodeInstance = true;
	bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_EjectVictim::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	APawn* AIPawn = AIController->GetPawn();
	if (!AIPawn) return EBTNodeResult::Failed;

	FVector FleeLoactaion = OwnerComp.GetBlackboardComponent()->GetValueAsVector(FleeLocationKey.SelectedKeyName);

	if (FVector::Dist(AIPawn->GetActorLocation(), FleeLoactaion) <= EjectTriggerDistance)
	{
		AHorrorCreature* Creature = Cast<AHorrorCreature>(AIPawn);
		if (Creature && Creature->GetSwallowedPlayer())
		{
			Creature->EjectPlayer(Creature->GetSwallowedPlayer());
		}
		return EBTNodeResult::Succeeded;
	}
	
	return EBTNodeResult::Failed;
}
