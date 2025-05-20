// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/TestPlayerCharacter.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISenseConfig_Sight.h"

ATestPlayerCharacter::ATestPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	StimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("Stimuli"));
	StimuliSource->RegisterForSense(UAISense_Sight::StaticClass());
	StimuliSource->bAutoRegister = true; // At GameStart, AutoRegister
	StimuliSource->RegisterWithPerceptionSystem();
}

void ATestPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

FGenericTeamId ATestPlayerCharacter::GetGenericTeamId() const
{
	return FGenericTeamId(2);
}


