#include "Missions/ItemCollectionMission.h"

#include "DataRow/MissionDataRow/ItemCollectMissionRow.h"

UItemCollectionMission::UItemCollectionMission()
{
	MissionType = EMissionType::ItemCollection;
}

void UItemCollectionMission::InitMission(const FMissionInitParams& Params, const uint8& NewMissionIndex)
{
	InitMission(Params, (EItemCollectMission)NewMissionIndex);
}

void UItemCollectionMission::InitMission(const FMissionInitParams& Params, const EItemCollectMission& NewMissionIndex)
{
	MissionType = Params.MissionType;
	GoalCount = Params.GoalCount;

	MissionName = Params.MissionName;
	MissionDescription = Params.MissionDescription;

	ExtraValues = Params.ExtraValues;

	MissionIndex = NewMissionIndex;
}

void UItemCollectionMission::BindDelegates(UObject* TargetForDelegate)
{
}

void UItemCollectionMission::UnbindDelegates(UObject* TargetForDelegate)
{
}

void UItemCollectionMission::OnConditionMet()
{
}

const uint8 UItemCollectionMission::GetMissionIndex() const
{
	return (uint8)MissionIndex;
}
