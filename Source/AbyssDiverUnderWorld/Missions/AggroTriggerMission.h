#pragma once

#include "CoreMinimal.h"
#include "Missions/MissionBase.h"
#include "GameplayTagContainer.h"
#include "Character/UnitBase.h"
#include "AggroTriggerMission.generated.h"

enum class EAggroTriggerMission : uint8;

struct FAggroMissionInitParams : public FMissionInitParams
{
	FAggroMissionInitParams
	(
		const EMissionType& InMissionType,
		const FString& InMissionName,
		const FString& InMissionDescription,
		const EMissionConditionType& InConditionType,
		const int32& InGoalCount,
		const TArray<int32>& InExtraValues,
		bool bInCompleteInstantly,
		bool InUseQuery,
		const FGameplayTag& InUnitIdTag,
		const FGameplayTag& InUnitTypeTag,
		const FGameplayTagQuery& InUnitTagQuery
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
		TargetUnitIdTag = InUnitIdTag;
		TargetUnitTypeTag = InUnitTypeTag;
		TargetUnitQuery = InUnitTagQuery;
	}
	bool bUseQuery = false;
	FGameplayTag TargetUnitIdTag;
	FGameplayTag TargetUnitTypeTag;
	FGameplayTagQuery TargetUnitQuery;

};

UCLASS()
class ABYSSDIVERUNDERWORLD_API UAggroTriggerMission : public UMissionBase
{
	GENERATED_BODY()
	
public:

	UAggroTriggerMission();

#pragma region Methods

public:

	virtual void InitMission(const FMissionInitParams& Params) override;
	void InitMission(const FAggroMissionInitParams& Params, const EAggroTriggerMission& NewMissionIndex);

	virtual void NotifyAggroTriggered(const FGameplayTagContainer& SourceTag) override;

#pragma endregion

#pragma region Variables
public:
	UPROPERTY(EditAnywhere)
	bool bUseQuery = false;

	UPROPERTY(EditAnywhere)
	FGameplayTag      TargetUnitIdTag;    // Unit.Id.Kraken

	UPROPERTY(EditAnywhere)
	FGameplayTag      TargetUnitTypeTag;  // Unit.Type.Shark

	UPROPERTY(EditAnywhere)
	FGameplayTagQuery TargetUnitQuery;    // ÇÊ¿ä ½Ã

protected:

	EAggroTriggerMission MissionIndex;

#pragma endregion


#pragma region Getters / Setters

public:
	virtual uint8 GetMissionIndex() const override;

#pragma endregion
};
