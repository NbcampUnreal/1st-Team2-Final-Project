// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BT/HorrorCreature/BTTask_EjectVictim.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Container/BlackboardKeys.h"
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

	TaskMemory->AIController = Cast<AMonsterAIController>(OwnerComp.GetAIOwner());
	TaskMemory->Monster = Cast<AMonster>(OwnerComp.GetAIOwner()->GetCharacter());
	
	if (!TaskMemory->AIController.IsValid() || !TaskMemory->Monster.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("EjectVictim_Task : AIController or Monster is InValid"));
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BB = TaskMemory->AIController->GetBlackboardComponent();
	if (BB)
	{
		TaskMemory->FleeLocation = BB->GetValueAsVector(BlackboardKeys::FleeLocationKey);
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_EjectVictim::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTEjectVictimTaskMemory* TaskMemory = (FBTEjectVictimTaskMemory*)NodeMemory;
	if (!TaskMemory) return;

	FVector MonsterLocation = TaskMemory->Monster->GetActorLocation();
	float Distance = FVector::Dist(MonsterLocation, TaskMemory->FleeLocation);

	TaskMemory->ElapsedTime += DeltaSeconds;
	UE_LOG(LogTemp, Log, TEXT("[EjectVictim] ElapsedTime: %.2f"), TaskMemory->ElapsedTime);

	if (Distance <= EjectTriggerDistance || TaskMemory->ElapsedTime >= MaxEjectDelay)
	{
		AHorrorCreature* HorrorCreature = Cast<AHorrorCreature>(TaskMemory->Monster);
		if (HorrorCreature && HorrorCreature->GetSwallowedPlayer())
		{
			HorrorCreature->EjectPlayer(HorrorCreature->GetSwallowedPlayer());
		}
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}