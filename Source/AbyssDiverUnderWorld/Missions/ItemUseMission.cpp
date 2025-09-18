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
	
	bUseQuery = Params.bUseQuery;
	TargetItemIdTag = Params.TargetItemIdTag;
	TargetItemTypeTag = Params.TargetItemTypeTag;
	TargetItemQuery = Params.TargetItemQuery;
}


void UItemUseMission::NotifyItemUsed(const FGameplayTagContainer& ItamTag, int32 Amount)
{
	if (IsCompleted()) 
		return;

	bool bMathched = false;
 	if (bUseQuery)
	{
		bMathched = TargetItemQuery.Matches(ItamTag);
	}
	else
	{
		if (TargetItemIdTag.IsValid())
		{
			bMathched = ItamTag.HasTag(TargetItemIdTag);
		}
		else if (TargetItemTypeTag.IsValid())
		{
			bMathched = ItamTag.HasTag(TargetItemTypeTag);
		}
	}

	AddProgress(FMath::Max(Amount, 1));
}


uint8 UItemUseMission::GetMissionIndex() const
{
	return uint8(MissionIndex);
}
