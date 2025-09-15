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

	TargetItemTag = Params.TargetItemTag;
	bIsOreMission = Params.bIsOreMission;
}

void UItemCollectionMission::BindDelegates(UObject* TargetForDelegate)
{
	if (bIsOreMission && TargetForDelegate->IsA<AADDroneSeller>())
	{
		/*AADDroneSeller* DroneSeller = Cast<AADDroneSeller>(TargetForDelegate);
		DroneSeller->OnSellOreDelegate.RemoveAll(this);
		DroneSeller->OnSellOreDelegate.AddUObject(this, &UItemCollectionMission::OnItemCollect);*/
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

void UItemCollectionMission::NotifyItemCollected(FGameplayTag ItemTag, int32 Amount)
{
	if (IsCompleted()) return;
	if (!ItemTag.MatchesTag(TargetItemTag)) return; // 정확 일치면 MatchesTagExact

	AddProgress(FMath::Max(Amount, 1));
}

uint8 UItemCollectionMission::GetMissionIndex() const
{
	return (uint8)MissionIndex;
}

FGameplayTag UItemCollectionMission::GetTargetItemTag() const
{
	return TargetItemTag;
}

const bool UItemCollectionMission::IsOreMission() const
{
	return bIsOreMission;
}
