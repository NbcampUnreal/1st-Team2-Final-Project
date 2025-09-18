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
	
	bUseQuery = Params.bUseQuery;
	TargetItemIdTag = Params.TargetItemIdTag;
	TargetItemTypeTag = Params.TargetItemTypeTag;
	TargetItemQuery = Params.TargetItemQuery;
}

void UItemCollectionMission::BindDelegates(UObject* TargetForDelegate)
{
	if (TargetForDelegate->IsA<AADDroneSeller>())
	{
		/*AADDroneSeller* DroneSeller = Cast<AADDroneSeller>(TargetForDelegate);
		DroneSeller->OnSellOreDelegate.RemoveAll(this);
		DroneSeller->OnSellOreDelegate.AddUObject(this, &UItemCollectionMission::OnItemCollect);*/
	}
}

void UItemCollectionMission::UnbindDelegates(UObject* TargetForDelegate)
{
	if (TargetForDelegate->IsA<AADDroneSeller>())
	{
		AADDroneSeller* DroneSeller = Cast<AADDroneSeller>(TargetForDelegate);
		DroneSeller->OnSellOreDelegate.RemoveAll(this);
	}
}

void UItemCollectionMission::NotifyItemCollected(const FGameplayTagContainer& ItemTag, int32 Amount)
{
	if (IsCompleted()) return;
	
	bool bMathched = false;
	if (bUseQuery)
	{
		bMathched = TargetItemQuery.Matches(ItemTag);
	}
	else
	{
		if (TargetItemIdTag.IsValid())
		{
			bMathched = ItemTag.HasTag(TargetItemIdTag);
		}
		else if (TargetItemTypeTag.IsValid())
		{
			bMathched = ItemTag.HasTag(TargetItemTypeTag);
		}
	}

	AddProgress(FMath::Max(Amount, 1));
}

uint8 UItemCollectionMission::GetMissionIndex() const
{
	return (uint8)MissionIndex;
}
