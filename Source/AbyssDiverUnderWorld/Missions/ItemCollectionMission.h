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

#pragma endregion

#pragma region Variables

protected:

	EItemCollectMission MissionIndex;

#pragma endregion


#pragma region Getters / Setters

public:

	virtual const uint8 GetMissionIndex() const override;

#pragma endregion
};
