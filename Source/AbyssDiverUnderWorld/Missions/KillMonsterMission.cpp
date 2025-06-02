#include "Missions/KillMonsterMission.h"

#include "DataRow/MissionDataRow/KillMonsterMissionRow.h"

UKillMonsterMission::UKillMonsterMission()
{
	MissionType = EMissionType::KillMonster;
}

void UKillMonsterMission::InitMission(const FMissionInitParams& Params, const uint8& NewMissionIndex)
{
	InitMission(Params, (EKillMonsterMission)NewMissionIndex);
}

void UKillMonsterMission::InitMission(const FMissionInitParams& Params, const EKillMonsterMission& NewMissionIndex)
{
	MissionType = Params.MissionType;
	GoalCount = Params.GoalCount;

	MissionName = Params.MissionName;
	MissionDescription = Params.MissionDescription;

	ExtraValues = Params.ExtraValues;

	MissionIndex = NewMissionIndex;
}

void UKillMonsterMission::BindDelegates(UObject* TargetForDelegate)
{
}

void UKillMonsterMission::OnConditionMet()
{
}

const uint8 UKillMonsterMission::GetMissionIndex() const
{
	return uint8(MissionIndex);
}
