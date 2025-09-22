#pragma once

#include "MissionBaseRow.h"
#include "Missions/MissionTagUtil.h"
#include "ItemCollectMissionRow.generated.h"

UENUM(BlueprintType)
enum class EItemCollectMission : uint8
{
	ItemMission1,
	ItemMission2,
	ItemMission3,
	ItemMission4,
	ItemMission5,
	MAX UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FItemCollectMissionRow : public FMissionBaseRow
{
	GENERATED_BODY()

	FItemCollectMissionRow()
	{
		MissionType = EMissionType::ItemCollection;
	}

	UPROPERTY(EditDefaultsOnly, Category = "ItemCollectMission")
	EItemCollectMission Mission;

	UPROPERTY(EditDefaultsOnly, Category = "ItemCollectMission")
	uint8 ItemId;

	UPROPERTY(EditDefaultsOnly, Category = "ItemCollectMission")
	uint8 bIsOreMission : 1;

	UPROPERTY(EditDefaultsOnly, Category = "ItemCollectMission|Tag")
	bool bUseQuery = false;

	UPROPERTY(EditDefaultsOnly, Category = "ItemCollectMission|Tag")
	FGameplayTagQuery TargetQuery;

	UPROPERTY(EditDefaultsOnly, Category = "ItemCollectMission|Tag")
	FName ItemTypeTail = NAME_None; // 예: "Consumable" 또는 "Ore"

	// ▼ 추가: 런타임 매칭용 태그
	UPROPERTY(VisibleAnywhere, Category = "ItemCollectMission|Tag")
	FGameplayTag TargetItemIdTag;

	UPROPERTY(VisibleAnywhere, Category = "ItemCollectMission|Tag")
	FGameplayTag TargetItemTypeTag;

	void BakeTags()
	{
		TargetItemIdTag = UMissionTagUtil::ToItemIdTagById(ItemId);

		// bIsOreMission을 타입 꼬리로 쓰고 싶다면(옵션)
		if (ItemTypeTail == NAME_None && bIsOreMission)
		{
			ItemTypeTail = FName(TEXT("Ore"));
		}

		if (ItemTypeTail != NAME_None)
		{
			TargetItemTypeTag = UMissionTagUtil::ToItemTypeTagByTail(ItemTypeTail);
		}
	}
};
