#include "Missions/MissionBase.h"

#include "DataRow/MissionDataRow/MissionBaseRow.h"

void UMissionBase::InitMission(const FMissionInitParams& Params)
{
	MissionType = Params.MissionType;
	GoalCount = Params.GoalCount;
	CurrentCount = 0;

	ConditionType = Params.ConditionType;

	MissionName = Params.MissionName;
	MissionDescription = Params.MissionDescription;

	ExtraValues = Params.ExtraValues;
}

void UMissionBase::OnConditionMet()
{
	OnCompleteMissionDelegate.ExecuteIfBound(MissionType, GetMissionIndex());
}
