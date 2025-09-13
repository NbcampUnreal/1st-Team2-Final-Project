#pragma once

#include "CoreMinimal.h"
#include "MissionEnum.generated.h"

UENUM(BlueprintType)
enum class EMissionType : uint8
{
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
	AtLeast, // GoalCount �̻��� ��� ���� ����
	AtMost, // GoalCount ������ ��� ���� ����
	EqualTo, // GoalCount�� ���� ��� ���� ����
	Custom, // ����
};

UENUM(BlueprintType)
enum class ELevelName : uint8
{
	None,
	ShallowWaterLevel,
	DeepWaterLevel,
};