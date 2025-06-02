#pragma once

#include "MissionBaseRow.h"

#include "KillMonsterMissionRow.generated.h"

UENUM(BlueprintType)
enum class EKillMonsterMission : uint8
{
	Mission1
};

enum class EUnitId : uint8;

USTRUCT(BlueprintType)
struct FKillMonsterMissionRow : public FMissionBaseRow
{
	GENERATED_BODY()

	FKillMonsterMissionRow()
	{
		MissionType = EMissionType::KillMonster;
	}
	
	UPROPERTY(EditDefaultsOnly, Category = "KillMonsterMission")
	EKillMonsterMission Mission;

	UPROPERTY(EditDefaultsOnly, Category = "KillMonsterMission")
	EUnitId UnitId;
};
