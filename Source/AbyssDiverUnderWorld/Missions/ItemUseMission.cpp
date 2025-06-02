#include "Missions/ItemUseMission.h"

#include "DataRow/MissionDataRow/ItemUseMissionRow.h"

UItemUseMission::UItemUseMission()
{
	MissionType = EMissionType::ItemUse;
}

void UItemUseMission::InitMission(const FMissionInitParams& Params)
{
	Super::InitMission(Params);
}

void UItemUseMission::InitMission(const FItemUseMissionInitParams& Params, const EItemUseMission& NewMissionIndex)
{
	InitMission((const FMissionInitParams&)Params);

	MissionIndex = NewMissionIndex;
}

void UItemUseMission::BindDelegates(UObject* TargetForDelegate)
{
}

void UItemUseMission::UnbindDelegates(UObject* TargetForDelegate)
{
}

bool UItemUseMission::IsConditionMet()
{
	return false;
}

const uint8 UItemUseMission::GetMissionIndex() const
{
	return uint8(MissionIndex);
}
