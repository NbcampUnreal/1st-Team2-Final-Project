#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "MissionBaseRow.generated.h"

UENUM(BlueprintType)
enum class EMissionType : uint8
{
	AggroTrigger,
	Interaction,
	ItemCollection,
	ItemUse,
	KillMonster
};

USTRUCT(BlueprintType)
struct FMissionBaseRow : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleDefaultsOnly, Category = "MissionBase")
	EMissionType MissionType;

	UPROPERTY(EditDefaultsOnly, Category = "MissionBase")
	int32 GoalCount;

	UPROPERTY(EditDefaultsOnly, Category = "MissionBase")
	FString MissionName;

	UPROPERTY(EditDefaultsOnly, Category = "MissionBase")
	FString MissionDescription;

	UPROPERTY(EditDefaultsOnly, Category = "MissionBase")
	FString UnlockHint;

	UPROPERTY(EditDefaultsOnly, Category = "MissionBase")
	int32 Stage;

	UPROPERTY(EditDefaultsOnly, Category = "MissionBase")
	TArray<int32> ExtraValues;

	// 처음에 미션이 잠금상태로 시작할 것인지 말 것인지 선택
	UPROPERTY(EditDefaultsOnly, Category = "MissionBase")
	uint8 bIsLocked : 1;
};
