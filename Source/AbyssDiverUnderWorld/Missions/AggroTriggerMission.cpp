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

void UAggroTriggerMission::NotifyAggroTriggered(const FGameplayTag& SourceTag)
{
    if (IsCompleted()) return;

    bool bMatched = false;
    if (bUseQuery)
    {
        FGameplayTagContainer Owned; Owned.AddTag(SourceTag);
        bMatched = TargetAggroQuery.Matches(Owned);
    }
    else
    {
        // ���� ���: Mission.Aggro.Enemy.Shark c Mission.Aggro.Enemy.*
        bMatched = SourceTag.MatchesTag(TargetTag);
        // ��Ȯ ��ġ ���ϸ� MatchesTagExact(TargetAggroTag)
    }
    if (!bMatched) return;

    AddProgress(1);
}

uint8 UAggroTriggerMission::GetMissionIndex() const
{
	return (uint8)MissionIndex;
}
