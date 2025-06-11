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

UENUM(BlueprintType)
enum class EMissionConditionType : uint8
{
	AtLeast, // GoalCount 이상일 경우 조건 만족
	AtMost, // GoalCount 이하일 경우 조건 만족
	EqualTo, // GoalCount와 같은 경우 조건 만족
	Custom, // 자율
};

USTRUCT(BlueprintType)
struct FMissionBaseRow : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleDefaultsOnly, Category = "MissionBase")
	EMissionType MissionType;

	UPROPERTY(EditDefaultsOnly, Category = "MissionBase")
	int32 GoalCount;

	// AtLeast : GoalCount 이상일 경우 조건 만족, AtMost : GoalCount 이하일 경우 조건 만족
	// EqualTo : GoalCount와 같은 경우 조건 만족, Custom : 자율
	UPROPERTY(EditDefaultsOnly, Category = "MissionBase")
	EMissionConditionType ConditionType;

	UPROPERTY(EditDefaultsOnly, Category = "MissionBase")
	FString MissionName;

	UPROPERTY(EditDefaultsOnly, Category = "MissionBase")
	FString MissionDescription;

	UPROPERTY(EditDefaultsOnly, Category = "MissionBase")
	FString UnlockHint;

	// 해금 조건 같은데 쓸 몇 스테이지 까지 갔는가 정도..
	UPROPERTY(EditDefaultsOnly, Category = "MissionBase")
	int32 Stage;

	UPROPERTY(EditDefaultsOnly, Category = "MissionBase")
	TArray<int32> ExtraValues;

	// 처음에 미션이 잠금상태로 시작할 것인지 말 것인지 선택
	UPROPERTY(EditDefaultsOnly, Category = "MissionBase")
	uint8 bIsLocked : 1;

	// 미션 완료 조건이 충족되는 즉시 미션 성공으로 간주할 것인가.
	UPROPERTY(EditDefaultsOnly, Category = "MissionBase")
	uint8 bShouldCompleteInstantly : 1;

	// HUD에 띄울 간략한 미션 이미지
	UPROPERTY(EditDefaultsOnly, Category = "MissionBase")
	TObjectPtr<UTexture2D> MissionImage;
};
