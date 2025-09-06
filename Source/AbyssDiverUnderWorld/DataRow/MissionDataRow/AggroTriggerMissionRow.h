#pragma once

#include "MissionBaseRow.h"

#include "AggroTriggerMissionRow.generated.h"

UENUM(BlueprintType)
enum class EAggroTriggerMission : uint8
{
	AggroMission1,
	AggroMission2,
	AggroMission3,
	AggroMission4,
	AggroMission5,
	MAX UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FAggroTriggerMissionRow : public FMissionBaseRow
{
	GENERATED_BODY()
	
	FAggroTriggerMissionRow()
	{
		MissionType = EMissionType::AggroTrigger;
	}

	UPROPERTY(EditDefaultsOnly, Category = "AggroTriggerMission")
	EAggroTriggerMission Mission;
};
