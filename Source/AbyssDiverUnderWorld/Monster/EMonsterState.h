// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EMonsterState.generated.h"

UENUM(BlueprintType)
enum class EMonsterState : uint8
{
	Idle = 0 UMETA(DisplayName = "Idle"),
	Patrol = 1 UMETA(DisplayName = "Patrol"),
	Detected = 2 UMETA(DisplayName = "Detected"),
	Chase = 3	UMETA(DisplayName = "Chase"),
	Investigate = 4 UMETA(DisplayName = "Investigate"),
	Attack = 5 UMETA(DisplayName = "Attack"),
	Flee = 6 UMETA(DisplayName = "Flee"),
	Death = 7 UMETA(DisplayName = "Death")
};
