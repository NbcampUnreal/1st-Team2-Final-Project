#pragma once

#include "CoreMinimal.h"
#include "Missions/MissionBase.h"
#include "GameplayTagContainer.h"

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
		const TArray<int32>& InExtraValues,
		bool bInCompleteInstantly
	)
		: FMissionInitParams(InMissionType, InMissionName, InMissionDescription, InConditionType, InGoalCount, InExtraValues, bInCompleteInstantly)
	{
	}

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

	virtual void NotifyAggroTriggered(const FGameplayTag& SourceTag) override;

#pragma endregion

#pragma region Variables
public:
	UPROPERTY(EditAnywhere)
	FGameplayTag TargetTag;

	UPROPERTY(EditAnywhere)
	FGameplayTagQuery TargetAggroQuery;

	UPROPERTY(EditAnywhere)
	uint8 bUseQuery : 1 = 0;;

protected:

	EAggroTriggerMission MissionIndex;

#pragma endregion


#pragma region Getters / Setters

public:
	virtual uint8 GetMissionIndex() const override;

#pragma endregion
};
