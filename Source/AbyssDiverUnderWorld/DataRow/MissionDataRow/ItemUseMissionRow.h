#pragma once

#include "MissionBaseRow.h"
#include "Missions/MissionTagUtil.h"
#include "ItemUseMissionRow.generated.h"

UENUM(BlueprintType)
enum class EItemUseMission : uint8
{
	ItemMission1,
	ItemMission2,
	ItemMission3,
	ItemMission4,
	ItemMission5,
	MAX UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FItemUseMissionRow : public FMissionBaseRow
{
	GENERATED_BODY()
	
	FItemUseMissionRow()
		: Mission(EItemUseMission::ItemMission1)
		, ItemId(0)
		, ItemTypeTail(NAME_None)
		, bUseQuery(false)
	{
		MissionType = EMissionType::ItemUse;
	}

	UPROPERTY(EditDefaultsOnly, Category = "ItemUseMission")
	EItemUseMission Mission;

	UPROPERTY(EditDefaultsOnly, Category = "ItemUseMission")
	uint8 ItemId;

	UPROPERTY(EditDefaultsOnly, Category = "ItemUseMission|Tag")
	FName ItemTypeTail = NAME_None;

	UPROPERTY(EditDefaultsOnly, Category = "ItemUseMission|Tag")
	bool bUseQuery = false;

	UPROPERTY(EditDefaultsOnly, Category = "ItemUseMission|Tag")
	FGameplayTagQuery TargetQuery;


	UPROPERTY(VisibleAnywhere, Category = "ItemUseMission|Tag")
	FGameplayTag TargetItemIdTag;

	UPROPERTY(VisibleAnywhere, Category = "ItemUseMission|Tag")
	FGameplayTag TargetItemTypeTag;

	void BakeTags()
	{
		TargetItemIdTag = UMissionTagUtil::ToItemIdTagById(ItemId);
		if (ItemTypeTail != NAME_None)
		{
			TargetItemTypeTag = UMissionTagUtil::ToItemTypeTagByTail(ItemTypeTail);
		}
	}
};
