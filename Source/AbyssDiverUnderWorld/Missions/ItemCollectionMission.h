#pragma once

#include "CoreMinimal.h"
#include "Missions/MissionBase.h"

#include "ItemCollectionMission.generated.h"

enum class EItemCollectMission : uint8;

struct FItemCollectMissionInitParams : public FMissionInitParams
{
	FItemCollectMissionInitParams
	(
		const EMissionType& InMissionType,
		const int32& InGoalCount,
		const EMissionConditionType& InConditionType,
		const FString& InMissionName,
		const FString& InMissionDescription,
		const TArray<int32>& InExtraValues,
		bool bInShouldCompleteInstanly,
		uint8 InTargetItemId,
		bool bInIsOreMission
	)
		: FMissionInitParams(InMissionType, InGoalCount, InConditionType, InMissionName, InMissionDescription, InExtraValues, bInShouldCompleteInstanly)
	{
		TargetItemId = InTargetItemId;
		bIsOreMission = bInIsOreMission;
	}

	uint8 TargetItemId;
	uint8 bIsOreMission : 1;

};

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UItemCollectionMission : public UMissionBase
{
	GENERATED_BODY()
	
public:

	UItemCollectionMission();

#pragma region Methods

public:

	virtual void InitMission(const FMissionInitParams& Params) override;
	void InitMission(const FItemCollectMissionInitParams& Params, const EItemCollectMission& NewMissionIndex);

	virtual void BindDelegates(UObject* TargetForDelegate) override;
	virtual void UnbindDelegates(UObject* TargetForDelegate) override;

protected:

	virtual bool IsConditionMet() override;

private:

	void OnItemCollect(uint8 ItemId, int32 Amount);

#pragma endregion

#pragma region Variables

protected:

	EItemCollectMission MissionIndex;
	uint8 TargetItemId;
	uint8 bIsOreMission : 1;

#pragma endregion


#pragma region Getters / Setters

public:

	virtual const uint8 GetMissionIndex() const override;
	const uint8 GetTargetItemId() const;
	const bool IsOreMission() const;

#pragma endregion
};
