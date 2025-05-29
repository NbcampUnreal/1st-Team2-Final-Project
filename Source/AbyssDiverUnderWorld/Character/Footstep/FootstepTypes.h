// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FootstepTypes.generated.h"

UENUM(BlueprintType)
enum class EFootSide : uint8
{
	None,
	Left,
	Right,
};

UENUM(BlueprintType)
enum class EFootstepType : uint8
{
	Walk,
	Land,
};
