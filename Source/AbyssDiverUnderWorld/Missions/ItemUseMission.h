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
		const int32& InGoalCount,
		const EMissionConditionType& InConditionType,
		const FString& InMissionName,
		const FString& InMissionDescription,
		const TArray<int32>& InExtraValues,
		const uint8 InTargetItemId,
		bool bInCompleteInstantly
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
		TargetItemId = InTargetItemId;
	}

	uint8 TargetItemId;
};

/**
 * 
 */
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

	virtual void NotifyItemUsed(uint8 ItemId, int32 Amount) override;

protected:


#pragma endregion

#pragma region Variables
public:
	UPROPERTY(EditAnywhere)
	uint8 TargetItemId = 0;


protected:

	EItemUseMission MissionIndex;

#pragma endregion


#pragma region Getters / Setters

public:

	virtual uint8 GetMissionIndex() const override;

#pragma endregion
};
