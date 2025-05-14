// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Tentacle/TentacleAIController.h"

ATentacleAIController::ATentacleAIController()
{
	MonsterID = "Tentacle";
}

void ATentacleAIController::BeginPlay()
{
	Super::BeginPlay();

	
}

void ATentacleAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}
