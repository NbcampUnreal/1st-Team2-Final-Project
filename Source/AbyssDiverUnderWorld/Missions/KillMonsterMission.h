#pragma once

#include "CoreMinimal.h"
#include "Missions/MissionBase.h"

#include "KillMonsterMission.generated.h"

enum class EKillMonsterMission : uint8;

class AMonster;

struct FKillMissionInitParams : public FMissionInitParams
{
	FKillMissionInitParams
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
		const FGameplayTagQuery& InUnitTagQuery,
		uint8 InNeededSimultaneousKillCount,
		float InKillInterval
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
		NeededSimultaneousKillCount = InNeededSimultaneousKillCount;
		KillInterval = InKillInterval;
	}
	bool bUseQuery = false;
	FGameplayTag TargetUnitIdTag;
	FGameplayTag TargetUnitTypeTag;
	FGameplayTagQuery TargetUnitQuery;

	uint8 NeededSimultaneousKillCount;
	float KillInterval;
};

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UKillMonsterMission : public UMissionBase
{
	GENERATED_BODY()
	
public:

	UKillMonsterMission();

#pragma region Methods

public:

	virtual void InitMission(const FMissionInitParams& Params) override;
	void InitMission(const FKillMissionInitParams& Params, const EKillMonsterMission& NewMissionIndex);


	virtual void NotifyMonsterKilled(const FGameplayTagContainer& UnitTags) override;

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


	UPROPERTY(EditAnywhere)
	int8 NeededSimultaneousKillCount = 1;
	UPROPERTY(EditAnywhere)
	float KillInterval = 0.1f;

protected:
	EKillMonsterMission MissionIndex;

	// LastKilledTime, CachedSimultaneousKillCount
	TArray<float> KillTimes;

#pragma endregion


#pragma region Getters / Setters

public:

	virtual uint8 GetMissionIndex() const override;

private:

	int32 GetPlayerIndex(AActor* DamageCauser);
	int32 GetPlayerIndexInternal(AActor* SuspectedPlayer);

#pragma endregion
};
