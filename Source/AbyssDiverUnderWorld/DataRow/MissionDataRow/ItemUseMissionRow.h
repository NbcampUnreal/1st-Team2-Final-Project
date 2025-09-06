#pragma once

#include "MissionBaseRow.h"

#include "ItemUseMissionRow.generated.h"

UENUM(BlueprintType)
enum class EItemUseMission : uint8
{
	ItemMission1,
	ItemMission2,
	ItemMission3,
	ItemMission4,
	ItemMission5,
	MAX UMETA(Hidden)
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

	UPROPERTY(EditDefaultsOnly, Category = "ItemUseMission")
	uint8 ItemId;
};
