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

    bUseQuery = Params.bUseQuery;
    TargetUnitIdTag = Params.TargetUnitIdTag;
    TargetUnitTypeTag = Params.TargetUnitTypeTag;
    TargetUnitQuery = Params.TargetUnitQuery;
}

void UAggroTriggerMission::NotifyAggroTriggered(const FGameplayTagContainer& SourceTag)
{
    if (IsCompleted()) return;

    bool bMatched = false;
    if (bUseQuery)
		bMatched = TargetUnitQuery.Matches(SourceTag);
	else if (TargetUnitIdTag.IsValid())
		bMatched = SourceTag.HasTagExact(TargetUnitIdTag);
	else if (TargetUnitTypeTag.IsValid())
		bMatched = SourceTag.HasTag(TargetUnitTypeTag);

    AddProgress(1);
}

uint8 UAggroTriggerMission::GetMissionIndex() const
{
	return (uint8)MissionIndex;
}
