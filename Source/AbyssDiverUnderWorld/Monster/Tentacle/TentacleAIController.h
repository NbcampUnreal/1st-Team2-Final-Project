// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Monster/MonsterAIController.h"
#include "TentacleAIController.generated.h"

UENUM(BlueprintType)
enum class ESearchState : uint8
{
	Idle = 0,
	Patrol = 1,
	Chasing = 2,
	Attack = 3
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API ATentacleAIController : public AMonsterAIController
{
	GENERATED_BODY()
	
public:
	

protected:


};
