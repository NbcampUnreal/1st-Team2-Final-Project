// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Monster/MonsterAIController.h"
#include "TentacleAIController.generated.h"


UCLASS()
class ABYSSDIVERUNDERWORLD_API ATentacleAIController : public AMonsterAIController
{
	GENERATED_BODY()
	
public:
	ATentacleAIController();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

};
