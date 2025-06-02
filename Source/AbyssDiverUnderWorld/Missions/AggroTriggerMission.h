#pragma once

#include "CoreMinimal.h"
#include "Missions/MissionBase.h"

#include "AggroTriggerMission.generated.h"

enum class EAggroTriggerMission : uint8;

struct FAggroMissionInitParams : public FMissionInitParams
{
	FAggroMissionInitParams
	(
		const EMissionType& InMissionType,
		const int32& InGoalCount,
		const EMissionConditionType& InConditionType,
		const FString& InMissionName,
		const FString& InMissionDescription,
		const TArray<int32>& InExtraValues
	)
		: FMissionInitParams(InMissionType, InGoalCount, InConditionType, InMissionName, InMissionDescription, InExtraValues)
	{
	}

};

/**
 * 
 */
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

	virtual void BindDelegates(UObject* TargetForDelegate) override;
	virtual void UnbindDelegates(UObject* TargetForDelegate) override;

protected:

	virtual bool IsConditionMet() override;

#pragma endregion

#pragma region Variables

protected:

	EAggroTriggerMission MissionIndex;

#pragma endregion


#pragma region Getters / Setters

public:

	virtual const uint8 GetMissionIndex() const override;

#pragma endregion
};
