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
	
	bUseQuery = Params.bUseQuery;
	TargetInteractionIdTag = Params.TargetInteractionIdTag;
	TargetInteractionTypeTag = Params.TargetInteractionTypeTag;
	TargetInteractionQuery = Params.TargetInteractionQuery;
}


void UInteractionMission::NotifyInteracted(const FGameplayTagContainer& InteractTags)
{
	if (IsCompleted()) return;

	bool bMathched = false;
	if (bUseQuery)
	{
		bMathched = TargetInteractionQuery.Matches(InteractTags);
	}
	else
	{
		if (TargetInteractionIdTag.IsValid())
		{
			bMathched = InteractTags.HasTag(TargetInteractionIdTag);
		}
		else if (TargetInteractionTypeTag.IsValid())
		{
			bMathched = InteractTags.HasTag(TargetInteractionTypeTag);
		}
	}

	AddProgress(1);
}

uint8 UInteractionMission::GetMissionIndex() const
{
	return (uint8)MissionIndex;
}
