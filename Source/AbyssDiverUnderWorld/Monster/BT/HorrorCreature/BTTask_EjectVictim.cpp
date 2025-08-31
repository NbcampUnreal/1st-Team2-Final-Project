// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BT/HorrorCreature/BTTask_EjectVictim.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Monster/HorrorCreature/HorrorCreature.h"

UBTTask_EjectVictim::UBTTask_EjectVictim()
{
	NodeName = "Eject Victim";
	bCreateNodeInstance = true;
	bNotifyTick = true;

	ElapsedTime = 0.0f;
	MaxEjectDelay = 5.0f;
}

EBTNodeResult::Type UBTTask_EjectVictim::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ElapsedTime = 0.0f;

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	APawn* AIPawn = AIController->GetPawn();
	if (!AIPawn) return EBTNodeResult::Failed;

	return EBTNodeResult::InProgress;
}

void UBTTask_EjectVictim::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* AIPawn = AIController ? AIController->GetPawn() : nullptr;
	if (!AIPawn)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	FVector FleeLocation = OwnerComp.GetBlackboardComponent()->GetValueAsVector(FleeLocationKey.SelectedKeyName);
	float Distance = FVector::Dist(AIPawn->GetActorLocation(), FleeLocation);

	ElapsedTime += DeltaSeconds;
	UE_LOG(LogTemp, Log, TEXT("[EjectVictim] ElapsedTime: %.2f"), ElapsedTime);

	if (Distance <= EjectTriggerDistance || ElapsedTime >= MaxEjectDelay)
	{
		EjectPlayerIfPossible(AIPawn);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

void UBTTask_EjectVictim::EjectPlayerIfPossible(APawn* AIPawn)
{
	AHorrorCreature* Creature = Cast<AHorrorCreature>(AIPawn);
	if (Creature && Creature->GetSwallowedPlayer())
	{
		Creature->EjectPlayer(Creature->GetSwallowedPlayer());
	}
}
