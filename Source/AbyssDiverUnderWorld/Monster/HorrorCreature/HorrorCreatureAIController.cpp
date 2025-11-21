// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/HorrorCreature/HorrorCreatureAIController.h"
#include "Monster/HorrorCreature/HorrorCreature.h"

AHorrorCreatureAIController::AHorrorCreatureAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	MonsterID = "HorrorCreature";
}

void AHorrorCreatureAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	AMonster* OwningMonster = GetOwningMonster();
	if (!OwningMonster) return;

	AHorrorCreature* HorrorMonster = Cast<AHorrorCreature>(OwningMonster);
	if (HorrorMonster && HorrorMonster->GetSwallowedPlayer())
	{
		// 만약 삼킨 상태라면 리턴
		return;
	}

	Super::OnTargetPerceptionUpdated(Actor, Stimulus);
}

