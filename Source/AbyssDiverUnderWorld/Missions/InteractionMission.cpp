#include "Missions/InteractionMission.h"

#include "DataRow/MissionDataRow/InteractionMissionRow.h"

UInteractionMission::UInteractionMission()
{
	MissionType = EMissionType::Interaction;
	
}

void UInteractionMission::InitMission(const FMissionInitParams& Params, const uint8& NewMissionIndex)
{
	InitMission(Params, (EInteractionMission)NewMissionIndex);
}

void UInteractionMission::InitMission(const FMissionInitParams& Params, const EInteractionMission& NewMissionIndex)
{
	MissionType = Params.MissionType;
	GoalCount = Params.GoalCount;

	MissionName = Params.MissionName;
	MissionDescription = Params.MissionDescription;

	ExtraValues = Params.ExtraValues;

	MissionIndex = NewMissionIndex;
}

void UInteractionMission::BindDelegates(UObject* TargetForDelegate)
{
}

void UInteractionMission::UnbindDelegates(UObject* TargetForDelegate)
{
}

void UInteractionMission::OnConditionMet()
{
}

const uint8 UInteractionMission::GetMissionIndex() const
{
	return (uint8)MissionIndex;
}
