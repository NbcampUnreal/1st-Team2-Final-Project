#pragma once

#include "MissionBaseRow.h"

#include "ItemCollectMissionRow.generated.h"

UENUM(BlueprintType)
enum class EItemCollectMission : uint8
{
	ItemMission1
};

USTRUCT(BlueprintType)
struct FItemCollectMissionRow : public FMissionBaseRow
{
	GENERATED_BODY()

	FItemCollectMissionRow()
	{
		MissionType = EMissionType::ItemCollection;
	}
	
	UPROPERTY(EditDefaultsOnly, Category = "ItemCollectMission")
	EItemCollectMission Mission;

	UPROPERTY(EditDefaultsOnly, Category = "ItemCollectMission")
	uint8 ItemId;

	UPROPERTY(EditDefaultsOnly, Category = "ItemCollectMission")
	uint8 bIsOreMission : 1;
};
