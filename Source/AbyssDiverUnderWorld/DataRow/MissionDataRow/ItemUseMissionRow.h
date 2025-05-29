#pragma once

#include "MissionBaseRow.h"

#include "ItemUseMissionRow.generated.h"

UENUM(BlueprintType)
enum class EItemUseMission : uint8
{
	ItemMission1
};

USTRUCT(BlueprintType)
struct FItemUseMissionRow : public FMissionBaseRow
{
	GENERATED_BODY()
	
	FItemUseMissionRow()
	{
		MissionType = EMissionType::ItemUse;
	}

	UPROPERTY(EditDefaultsOnly, Category = "ItemUseMission")
	EItemUseMission Mission;
};
