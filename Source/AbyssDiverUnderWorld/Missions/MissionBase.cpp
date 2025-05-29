#include "Missions/MissionBase.h"

#include "DataRow/MissionDataRow/MissionBaseRow.h"

void UMissionBase::InitMission(const FMissionInitParams& Params)
{
	MissionType = Params.MissionType;
	GoalCount = Params.GoalCount;

	MissionName = Params.MissionName;
	MissionDescription = Params.MissionDescription;

	ExtraValues = Params.ExtraValues;
}
