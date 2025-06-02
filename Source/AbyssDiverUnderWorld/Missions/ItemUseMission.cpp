#include "Missions/ItemUseMission.h"

#include "DataRow/MissionDataRow/ItemUseMissionRow.h"

UItemUseMission::UItemUseMission()
{
	MissionType = EMissionType::ItemUse;
}

void UItemUseMission::InitMission(const FMissionInitParams& Params, const uint8& NewMissionIndex)
{
	InitMission(Params, (EItemUseMission)NewMissionIndex);
}

void UItemUseMission::InitMission(const FMissionInitParams& Params, const EItemUseMission& NewMissionIndex)
{
	MissionType = Params.MissionType;
	GoalCount = Params.GoalCount;

	MissionName = Params.MissionName;
	MissionDescription = Params.MissionDescription;

	ExtraValues = Params.ExtraValues;

	MissionIndex = NewMissionIndex;
}

void UItemUseMission::BindDelegates(UObject* TargetForDelegate)
{
}

void UItemUseMission::UnbindDelegates(UObject* TargetForDelegate)
{
}

void UItemUseMission::OnConditionMet()
{
}

const uint8 UItemUseMission::GetMissionIndex() const
{
	return uint8(MissionIndex);
}
