// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/** 캐릭터의 이동 상태를 나타낸다. 현재는 Fall 상태와 Jump 상태를 구분하기 위해 사용한다. */
UENUM(BlueprintType)
enum class ELocomotionMode : uint8
{
	None UMETA(DisplayName = "None"),
	Jumping UMETA(DisplayName = "Jumping"),
};

UENUM(BlueprintType)
enum class EMoveDirection : uint8
{
	Other UMETA(DisplayName = "Other"),
	Forward UMETA(DisplayName = "Forward"),
	Backward UMETA(DisplayName = "Backward"),
};
