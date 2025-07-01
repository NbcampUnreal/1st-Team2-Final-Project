// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Monster/MonsterAIController.h"
#include "HorrorCreatureAIController.generated.h"

class USphereComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AHorrorCreatureAIController : public AMonsterAIController
{
	GENERATED_BODY()


public:
	AHorrorCreatureAIController();

	virtual void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus) override;
};
