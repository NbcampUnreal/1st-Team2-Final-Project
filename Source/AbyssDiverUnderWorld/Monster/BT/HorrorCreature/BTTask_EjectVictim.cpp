// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BT/HorrorCreature/BTTask_EjectVictim.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Container/BlackboardKeys.h"
#include "Monster/HorrorCreature/HorrorCreatureAIController.h"
#include "Monster/HorrorCreature/HorrorCreature.h"

UBTTask_EjectVictim::UBTTask_EjectVictim()
{
	NodeName = "Eject Victim";
	bCreateNodeInstance = false;
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_EjectVictim::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTEjectVictimTaskMemory* TaskMemory = (FBTEjectVictimTaskMemory*)NodeMemory;
	if (!TaskMemory) return EBTNodeResult::Failed;

	TaskMemory->HCAIController = Cast<AHorrorCreatureAIController>(OwnerComp.GetAIOwner());
	TaskMemory->HorrorCreature = Cast<AHorrorCreature>(OwnerComp.GetAIOwner()->GetCharacter());
	
	if (!TaskMemory->HCAIController.IsValid() || !TaskMemory->HorrorCreature.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("EjectVictim_Task : AIController or Monster is InValid"));
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BB = TaskMemory->HCAIController->GetBlackboardComponent();
	if (BB)
	{
		TaskMemory->FleeLocation = BB->GetValueAsVector(BlackboardKeys::HorrorCreature::FleeLocationKey);
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_EjectVictim::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTEjectVictimTaskMemory* TaskMemory = (FBTEjectVictimTaskMemory*)NodeMemory;
	if (!TaskMemory) return;

	FVector MonsterLocation = TaskMemory->HorrorCreature->GetActorLocation();
	float Distance = FVector::Dist(MonsterLocation, TaskMemory->FleeLocation);

	if (Distance <= EjectTriggerDistance)
	{
		if (TaskMemory->HorrorCreature.IsValid() && TaskMemory->HorrorCreature->GetSwallowedPlayer() != nullptr)
		{
			TaskMemory->HorrorCreature->EjectPlayer(TaskMemory->HorrorCreature->GetSwallowedPlayer());
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
		else
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		}
	}
}

uint16 UBTTask_EjectVictim::GetInstanceMemorySize() const
{
	return sizeof(FBTEjectVictimTaskMemory);
}
