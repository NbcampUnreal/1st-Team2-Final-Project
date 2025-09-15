#include "Missions/InteractionMission.h"

#include "DataRow/MissionDataRow/InteractionMissionRow.h"

UInteractionMission::UInteractionMission()
{
	MissionType = EMissionType::Interaction;
	
}

void UInteractionMission::InitMission(const FMissionInitParams& Params)
{
	Super::InitMission(Params);;
}

void UInteractionMission::InitMission(const FInteractiontMissionInitParams& Params, const EInteractionMission& NewMissionIndex)
{
	InitMission((const FMissionInitParams&)Params);
	MissionIndex = NewMissionIndex;
	TargetInteractionTag = Params.TargetInteractionTag;
}


void UInteractionMission::NotifyInteracted(FGameplayTag Tag)
{
	if (IsCompleted()) return;

	if (!Tag.MatchesTag(TargetInteractionTag))
		return;

	AddProgress(1);
}

uint8 UInteractionMission::GetMissionIndex() const
{
	return (uint8)MissionIndex;
}
