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
	TargetItemId = Params.TargetItemId;
}


void UItemUseMission::NotifyItemUsed(uint8 ItemId, int32 Amount)
{
	if (IsCompleted()) 
		return;

	if (ItemId != TargetItemId)
		return;

	AddProgress(FMath::Max(Amount, 1));
}


uint8 UItemUseMission::GetMissionIndex() const
{
	return uint8(MissionIndex);
}
