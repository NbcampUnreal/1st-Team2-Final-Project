#include "Missions/InteractionMission.h"

#include "DataRow/MissionDataRow/InteractionMissionRow.h"

UInteractionMission::UInteractionMission()
{
	MissionType = EMissionType::Interaction;
	
}

void UInteractionMission::InitMission(const FMissionInitParams& Params)
{
	Super::InitMission(Params);
}

void UInteractionMission::InitMission(const FInteractiontMissionInitParams& Params, const EInteractionMission& NewMissionIndex)
{
	InitMission((const FMissionInitParams&)Params);
	MissionIndex = NewMissionIndex;
}

void UInteractionMission::BindDelegates(UObject* TargetForDelegate)
{
}

void UInteractionMission::UnbindDelegates(UObject* TargetForDelegate)
{
}

bool UInteractionMission::IsConditionMet()
{
	return false;
}

const uint8 UInteractionMission::GetMissionIndex() const
{
	return (uint8)MissionIndex;
}
