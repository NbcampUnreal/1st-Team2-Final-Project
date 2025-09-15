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
		const FString& InMissionName,
		const FString& InMissionDescription,
		const EMissionConditionType& InConditionType,
		const int32& InGoalCount,
		FGameplayTag InTargetItemTag,
		bool bInIsOreMission,
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
		TargetItemTag = InTargetItemTag;
		bIsOreMission = bInIsOreMission;
	}

	FGameplayTag TargetItemTag;
	uint8 bIsOreMission : 1;
};

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

	virtual void NotifyItemCollected(FGameplayTag ItemTag, int32 Amount) override;

#pragma endregion

#pragma region Variables
public:
	UPROPERTY(EditAnywhere)
	FGameplayTag TargetItemTag;
protected:

	EItemCollectMission MissionIndex;
	uint8 bIsOreMission : 1;

#pragma endregion

#pragma region Getters / Setters

public:
	virtual uint8 GetMissionIndex() const override;
	FGameplayTag GetTargetItemTag() const;
	const bool IsOreMission() const;

#pragma endregion
};
