#pragma once

#include "MissionBaseRow.h"
#include "Missions/MissionTagUtil.h"
#include "AggroTriggerMissionRow.generated.h"

UENUM(BlueprintType)
enum class EAggroTriggerMission : uint8
{
	AggroMission1,
	AggroMission2,
	AggroMission3,
	AggroMission4,
	AggroMission5,
	MAX UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FAggroTriggerMissionRow : public FMissionBaseRow
{
	GENERATED_BODY()
	
	FAggroTriggerMissionRow()
		:Mission(EAggroTriggerMission::AggroMission1)
		, bUseQuery(false)
		, SourceUnitId(static_cast<EUnitId>(0)) // 프로젝트의 None 값으로 교체
		, SourceUnitTypeTail(NAME_None)
	{
		MissionType = EMissionType::AggroTrigger;
	}

	UPROPERTY(EditDefaultsOnly, Category = "AggroTriggerMission")

	EAggroTriggerMission Mission;

	UPROPERTY(EditDefaultsOnly, Category = "KillMonsterMission|Tag")
	bool bUseQuery = false;

	UPROPERTY(EditDefaultsOnly, Category = "KillMonsterMission|Tag")
	FGameplayTagQuery TargetQuery;

	UPROPERTY(EditDefaultsOnly, Category = "AggroTriggerMission|Tag")
	EUnitId SourceUnitId = static_cast<EUnitId>(0); // 프로젝트의 None 값으로 교체

	// ▼ 추가(선택): 어그로 소스 타입
	UPROPERTY(EditDefaultsOnly, Category = "AggroTriggerMission|Tag")
	FName SourceUnitTypeTail = NAME_None; // 예: "Shark"

	UPROPERTY(VisibleAnywhere, Category = "AggroTriggerMission|Tag")
	FGameplayTag TargetSourceUnitTypeTag;

	UPROPERTY(VisibleAnywhere, Category = "AggroTriggerMission|Tag")
	FGameplayTag TargetSourceUnitIdTag;

	void BakeTags()
	{
		TargetSourceUnitIdTag = UMissionTagUtil::ToUnitIdTag(SourceUnitId);

		// Type → Tag (선택)
		if (SourceUnitTypeTail != NAME_None)
		{
			TargetSourceUnitTypeTag = UMissionTagUtil::ToUnitTypeTagByTail(SourceUnitTypeTail);
		}
	}
};
