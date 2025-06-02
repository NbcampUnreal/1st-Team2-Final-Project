#include "Missions/AggroTriggerMission.h"

#include "DataRow/MissionDataRow/AggroTriggerMissionRow.h"

UAggroTriggerMission::UAggroTriggerMission()
{
    MissionType = EMissionType::AggroTrigger;
}

void UAggroTriggerMission::InitMission(const FMissionInitParams& Params, const uint8& NewMissionIndex)
{
	InitMission(Params, (EAggroTriggerMission)NewMissionIndex);
}

void UAggroTriggerMission::InitMission(const FMissionInitParams& Params, const EAggroTriggerMission& NewMissionIndex)
{
	MissionType = Params.MissionType;
	GoalCount = Params.GoalCount;
	ConditionType = Params.ConditionType;

	MissionName = Params.MissionName;
	MissionDescription = Params.MissionDescription;

	ExtraValues = Params.ExtraValues;

	MissionIndex = NewMissionIndex;
}

void UAggroTriggerMission::BindDelegates(UObject* TargetForDelegate)
{
}

void UAggroTriggerMission::UnbindDelegates(UObject* TargetForDelegate)
{
}

bool UAggroTriggerMission::IsConditionMet()
{
	return false;
}

void UAggroTriggerMission::OnConditionMet()
{
}

const uint8 UAggroTriggerMission::GetMissionIndex() const
{
	return (uint8)MissionIndex;
}
