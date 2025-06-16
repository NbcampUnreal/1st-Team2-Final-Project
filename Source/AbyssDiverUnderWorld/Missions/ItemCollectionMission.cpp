#include "Missions/ItemCollectionMission.h"

#include "DataRow/MissionDataRow/ItemCollectMissionRow.h"
#include "Interactable/OtherActors/ADDroneSeller.h"

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

	TargetItemId = Params.TargetItemId;
	bIsOreMission = Params.bIsOreMission;
}

void UItemCollectionMission::BindDelegates(UObject* TargetForDelegate)
{
	if (bIsOreMission && TargetForDelegate->IsA<AADDroneSeller>())
	{
		AADDroneSeller* DroneSeller = Cast<AADDroneSeller>(TargetForDelegate);
		DroneSeller->OnSellOreDelegate.RemoveAll(this);
		DroneSeller->OnSellOreDelegate.AddUObject(this, &UItemCollectionMission::OnItemCollect);
	}
}

void UItemCollectionMission::UnbindDelegates(UObject* TargetForDelegate)
{
	if (bIsOreMission && TargetForDelegate->IsA<AADDroneSeller>())
	{
		AADDroneSeller* DroneSeller = Cast<AADDroneSeller>(TargetForDelegate);
		DroneSeller->OnSellOreDelegate.RemoveAll(this);
	}
}

bool UItemCollectionMission::IsConditionMet()
{
	return GoalCount <= CurrentCount;
}

void UItemCollectionMission::OnItemCollect(uint8 ItemId, int32 Amount)
{
	if (ItemId != TargetItemId)
	{
		return;
	}

	if (bIsOreMission)
	{
		int32 Mass = Amount;
		CurrentCount += Mass;
	}
	else
	{
		CurrentCount += Amount;
	}

	if (IsConditionMet() == false)
	{
		return;
	}

	OnConditionMet();
}

const uint8 UItemCollectionMission::GetMissionIndex() const
{
	return (uint8)MissionIndex;
}

const uint8 UItemCollectionMission::GetTargetItemId() const
{
	return TargetItemId;
}

const bool UItemCollectionMission::IsOreMission() const
{
	return bIsOreMission;
}
