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
		const TArray<int32>& InExtraValues,
		bool bInCompleteInstantly,
		bool InUseQuery,
		const FGameplayTag& InInteractionIdTag,
		const FGameplayTag& InInteractionTypeTag,
		const FGameplayTagQuery& InInteractionTagQuery
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
		TargetInteractionIdTag = InInteractionIdTag;
		TargetInteractionTypeTag = InInteractionTypeTag;
		TargetInteractionQuery = InInteractionTagQuery;
	}

	bool bUseQuery = false;
	FGameplayTag TargetInteractionIdTag;
	FGameplayTag TargetInteractionTypeTag;
	FGameplayTagQuery TargetInteractionQuery;
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

	virtual void NotifyInteracted(const FGameplayTagContainer& InteractTags) override;

#pragma endregion

#pragma region Variables
public:
	UPROPERTY(EditAnywhere)
	bool bUseQuery = false;

	UPROPERTY(EditAnywhere)
	FGameplayTag      TargetInteractionIdTag;    // Unit.Id.Kraken

	UPROPERTY(EditAnywhere)
	FGameplayTag      TargetInteractionTypeTag;  // Unit.Type.Shark

	UPROPERTY(EditAnywhere)
	FGameplayTagQuery TargetInteractionQuery;    // ÇÊ¿ä ½Ã

protected:

	EInteractionMission MissionIndex;

#pragma endregion


#pragma region Getters / Setters

public:

	virtual uint8 GetMissionIndex() const override;

#pragma endregion
	
};
