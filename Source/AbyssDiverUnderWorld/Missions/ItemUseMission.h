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
		bool bInShouldCompleteInstanly
	)
		: FMissionInitParams(InMissionType, InGoalCount, InConditionType, InMissionName, InMissionDescription, InExtraValues, bInShouldCompleteInstanly)
	{
	}

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

	virtual void BindDelegates(UObject* TargetForDelegate) override;
	virtual void UnbindDelegates(UObject* TargetForDelegate) override;

protected:

	virtual bool IsConditionMet() override;

#pragma endregion

#pragma region Variables

protected:

	EItemUseMission MissionIndex;

#pragma endregion


#pragma region Getters / Setters

public:

	virtual const uint8 GetMissionIndex() const override;

#pragma endregion
};
