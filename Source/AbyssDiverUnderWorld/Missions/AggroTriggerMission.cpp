#include "Missions/AggroTriggerMission.h"

#include "DataRow/MissionDataRow/AggroTriggerMissionRow.h"

UAggroTriggerMission::UAggroTriggerMission()
{
    MissionType = EMissionType::AggroTrigger;
}

void UAggroTriggerMission::InitMission(const FMissionInitParams& Params)
{
	Super::InitMission(Params);
}

void UAggroTriggerMission::InitMission(const FAggroMissionInitParams& Params, const EAggroTriggerMission& NewMissionIndex)
{
	InitMission((const FMissionInitParams&)Params);

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

const uint8 UAggroTriggerMission::GetMissionIndex() const
{
	return (uint8)MissionIndex;
}
