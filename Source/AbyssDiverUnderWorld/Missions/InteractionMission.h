#pragma once

#include "CoreMinimal.h"
#include "Missions/MissionBase.h"

#include "InteractionMission.generated.h"

enum class EInteractionMission : uint8;

struct FInteractiontMissionInitParams : public FMissionInitParams
{
	FInteractiontMissionInitParams
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
class ABYSSDIVERUNDERWORLD_API UInteractionMission : public UMissionBase
{
	GENERATED_BODY()

public:

	UInteractionMission();

#pragma region Methods
public:

	virtual void InitMission(const FMissionInitParams& Params) override;
	void InitMission(const FInteractiontMissionInitParams& Params, const EInteractionMission& NewMissionIndex);

	virtual void BindDelegates(UObject* TargetForDelegate) override;
	virtual void UnbindDelegates(UObject* TargetForDelegate) override;

protected:

	virtual bool IsConditionMet() override;

#pragma endregion

#pragma region Variables

protected:

	EInteractionMission MissionIndex;

#pragma endregion


#pragma region Getters / Setters

public:

	virtual const uint8 GetMissionIndex() const override;

#pragma endregion
	
};
