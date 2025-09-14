#pragma once

#include "CoreMinimal.h"
#include "Missions/MissionBase.h"
#include "GameplayTagContainer.h"

#include "InteractionMission.generated.h"

enum class EInteractionMission : uint8;

struct FInteractiontMissionInitParams : public FMissionInitParams
{
	FInteractiontMissionInitParams
	(
		const EMissionType& InMissionType,
		const FString& InMissionName,
		const FString& InMissionDescription,
		const EMissionConditionType& InConditionType,
		const int32& InGoalCount,
		const FGameplayTag InTargetInteractionTag,
		const TArray<int32>& InExtraValues,
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
		TargetInteractionTag = InTargetInteractionTag;
	}

	FGameplayTag TargetInteractionTag;
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

	virtual void NotifyInteracted(FGameplayTag Tag) override;

#pragma endregion

#pragma region Variables
public:
	UPROPERTY(EditAnywhere)
	FGameplayTag TargetInteractionTag;
protected:

	EInteractionMission MissionIndex;

#pragma endregion


#pragma region Getters / Setters

public:

	virtual uint8 GetMissionIndex() const override;

#pragma endregion
	
};
