#include "Missions/ItemCollectionMission.h"

#include "DataRow/MissionDataRow/ItemCollectMissionRow.h"

UItemCollectionMission::UItemCollectionMission()
{
	MissionType = EMissionType::ItemCollection;
}

void UItemCollectionMission::InitMission(const FMissionInitParams& Params)
{
	Super::InitMission(Params);
}

void UItemCollectionMission::InitMission(const FItemCollectMissionInitParams& Params, const EItemCollectMission& NewMissionIndex)
{
	InitMission((const FMissionInitParams&)Params);

	MissionIndex = NewMissionIndex;
}

void UItemCollectionMission::BindDelegates(UObject* TargetForDelegate)
{
}

void UItemCollectionMission::UnbindDelegates(UObject* TargetForDelegate)
{
}

bool UItemCollectionMission::IsConditionMet()
{
	return false;
}

const uint8 UItemCollectionMission::GetMissionIndex() const
{
	return (uint8)MissionIndex;
}
