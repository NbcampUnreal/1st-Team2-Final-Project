#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "MissionBase.generated.h"

enum class EMissionType : uint8;

struct FMissionInitParams
{
	FMissionInitParams(const EMissionType& InMissionType, const int32& InGoalCount, const FString& InMissionName, const FString& InMissionDescription, const TArray<int32>& InExtraValues)
	{
		MissionType = InMissionType;
		GoalCount = InGoalCount;
		MissionName = InMissionName;
		MissionDescription = InMissionDescription;
		ExtraValues = InExtraValues;
	}

	EMissionType MissionType;
	int32 GoalCount;
	FString MissionName;
	FString MissionDescription;
	TArray<int32> ExtraValues;
};

/**
 * 
 */
UCLASS(Abstract)
class ABYSSDIVERUNDERWORLD_API UMissionBase : public UObject
{
	GENERATED_BODY()
	
#pragma region Methods

public:

	void InitMission(const FMissionInitParams& Params);
	virtual void BindDelegates(UObject* TargetForDelegate) PURE_VIRTUAL(UMissionBase::BindDelegates, );

protected:

	virtual void OnConditionMet() PURE_VIRTUAL(UMissionBase::OnConditionMet, );

#pragma endregion

#pragma region Variables

protected:


	EMissionType MissionType;

	int32 GoalCount;
	int32 CurrentCount;

	FString MissionName;
	FString MissionDescription;

	TArray<int32> ExtraValues;

#pragma endregion

#pragma region Getters / Setters

public:
	//FORCEINLINE const int32 GetGoalCount() 
	FORCEINLINE const FString& GetMissionName() const { return MissionName; }

#pragma endregion

};
