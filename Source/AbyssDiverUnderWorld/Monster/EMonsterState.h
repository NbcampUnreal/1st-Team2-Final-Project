// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EMonsterState.generated.h"

UENUM(BlueprintType)
enum class EMonsterState : uint8
{
	Idle = 0 UMETA(DisplayName = "Idle"),
	Patrol = 1 UMETA(DisplayName = "Patrol"),
	Chase = 2	UMETA(DisplayName = "Chasing"),
	Attack = 3 UMETA(DisplayName = "Attacking"),
	Dead = 4 UMETA(DisplayName = "Dead")
};
