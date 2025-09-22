#pragma once

#include "MissionBaseRow.h"
#include "Missions/MissionTagUtil.h"
#include "InteractionMissionRow.generated.h"

UENUM(BlueprintType)
enum class EInteractionMission : uint8
{
	InteractionMission1,
	InteractionMission2,
	InteractionMission3,
	InteractionMission4,
	InteractionMission5,
	MAX UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FInteractionMissionRow : public FMissionBaseRow
{
	GENERATED_BODY()

	FInteractionMissionRow()
	{
		MissionType = EMissionType::Interaction;
	}
	
	UPROPERTY(EditDefaultsOnly, Category = "InteractionMission")
	EInteractionMission Mission;

	UPROPERTY(EditDefaultsOnly, Category = "InteractionMission|Tag")
	bool bUseQuery = false;

	UPROPERTY(EditDefaultsOnly, Category = "InteractionMission|Tag")
	FGameplayTagQuery TargetQuery;

	UPROPERTY(EditDefaultsOnly, Category = "InteractionMission|Tag")
	FName InteractTypeTail = NAME_None; // 예: "Terminal", "Door"

	UPROPERTY(VisibleAnywhere, Category = "InteractionMission|Tag")
	FGameplayTag TargetInteractTypeTag;

	UPROPERTY(VisibleAnywhere, Category = "InteractionMission|Tag")
	FGameplayTag TargetInteractUnitIdTag;

	void BakeTags()
	{
		if (InteractTypeTail != NAME_None)
		{
			TargetInteractTypeTag = UMissionTagUtil::ToUnitTypeTagByTail(InteractTypeTail);
			// 상호작용 대상이 '아이템' 계열이면 ToItemTypeTagByTail로 바꾸면 됨.
		}
	}
};
