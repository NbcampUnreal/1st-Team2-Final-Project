#pragma once

#include "CoreMinimal.h"
#include "MissionEnum.generated.h"

UENUM(BlueprintType)
enum class EMissionType : uint8
{
	None,
	AggroTrigger,
	Interaction,
	ItemCollection,
	ItemUse,
	KillMonster,
	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EMissionConditionType : uint8
{
	AtLeast, // GoalCount 이상일 경우 조건 만족
	AtMost, // GoalCount 이하일 경우 조건 만족
	EqualTo, // GoalCount와 같은 경우 조건 만족
	Custom, // 자율
};

UENUM(BlueprintType)
enum class ELevelName : uint8
{
	None UMETA(DisplayName = "None"),

	TutorialPool UMETA(DisplayName = "Tutorial Pool"),
	SecondAbyss UMETA(DisplayName = "Second Abyss"),
	DeepAbyss UMETA(DisplayName = "Deep Abyss"),
};
