#pragma once

#include "MissionBaseRow.h"

#include "KillMonsterMissionRow.generated.h"

UENUM(BlueprintType)
enum class EKillMonsterMission : uint8
{
	KillMission1,
	KillMission2,
	KillMission3,
	KillMission4,
	KillMission5,
	KillMission6,
	KillMission7,
	KillMission8,
	KillMission9,
	KillMission10,
	KillMission11,
	KillMission12,
	KillMission13,
	KillMission14,
	KillMission15,
	KillMission16,
	MAX UMETA(Hidden)
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

	UPROPERTY(EditDefaultsOnly, Category = "KillMonsterMission")
	uint8 NeededSimultaneousKillCount = 1;

	UPROPERTY(EditDefaultsOnly, Category = "KillMonsterMission")
	float KillInterval;
};
