#pragma once

#include "CoreMinimal.h"
#include "Missions/MissionBase.h"

#include "ItemUseMission.generated.h"

enum class EItemUseMission : uint8;

struct FItemUseMissionInitParams : public FMissionInitParams
{
	FItemUseMissionInitParams
	(
		const EMissionType& InMissionType,
		const FString& InMissionName,
		const FString& InMissionDescription,
		const EMissionConditionType& InConditionType,
		const int32& InGoalCount,
		const TArray<int32>& InExtraValues,
		bool bInCompleteInstantly,
		bool InUseQuery,
		const FGameplayTag& InItemIdTag,
		const FGameplayTag& InItemTypeTag,
		const FGameplayTagQuery& InItemTagQuery
	)
		: FMissionInitParams
		(
			InMissionType,
			InMissionName,
			InMissionDescription,
			InConditionType,
			InGoalCount,
			InExtraValues,
			bInCompleteInstantly
		)
	{
		bUseQuery = InUseQuery;
		TargetItemIdTag = InItemIdTag;
		TargetItemTypeTag = InItemTypeTag;
		TargetItemQuery = InItemTagQuery;
	}
	bool bUseQuery = false;
	FGameplayTag TargetItemIdTag;
	FGameplayTag TargetItemTypeTag;
	FGameplayTagQuery TargetItemQuery;
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API UItemUseMission : public UMissionBase
{
	GENERATED_BODY()
	
public:
	UItemUseMission();

#pragma region Methods

public:
	virtual void InitMission(const FMissionInitParams& Params) override;
	void InitMission(const FItemUseMissionInitParams& Params, const EItemUseMission& NewMissionIndex);

	virtual void NotifyItemUsed(const FGameplayTagContainer& ItemTags, int32 Amount) override;

protected:

#pragma endregion

#pragma region Variables
public:
	UPROPERTY(EditAnywhere)
	bool bUseQuery = false;

	UPROPERTY(EditAnywhere)
	FGameplayTag      TargetItemIdTag;    // Unit.Id.Kraken

	UPROPERTY(EditAnywhere)
	FGameplayTag      TargetItemTypeTag;  // Unit.Type.Shark

	UPROPERTY(EditAnywhere)
	FGameplayTagQuery TargetItemQuery;    // ÇÊ¿ä ½Ã

protected:
	EItemUseMission MissionIndex;

#pragma endregion

#pragma region Getters / Setters
public:
	virtual uint8 GetMissionIndex() const override;

#pragma endregion
};
