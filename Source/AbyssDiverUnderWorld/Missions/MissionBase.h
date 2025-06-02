#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "MissionBase.generated.h"

enum class EMissionType : uint8;
enum class EMissionConditionType : uint8;

DECLARE_DELEGATE_TwoParams(FOnCompleteMissionDelegate, const EMissionType&, const uint8& MissionIndex);

struct FMissionInitParams
{
	FMissionInitParams(const EMissionType& InMissionType, const int32& InGoalCount, const EMissionConditionType& InConditionType, const FString& InMissionName, const FString& InMissionDescription, const TArray<int32>& InExtraValues)
	{
		MissionType = InMissionType;
		GoalCount = InGoalCount;
		ConditionType = InConditionType;
		MissionName = InMissionName;
		MissionDescription = InMissionDescription;
		ExtraValues = InExtraValues;
	}

	EMissionType MissionType;
	int32 GoalCount;
	EMissionConditionType ConditionType;
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

	virtual void InitMission(const FMissionInitParams& Params);
	virtual void BindDelegates(UObject* TargetForDelegate) PURE_VIRTUAL(UMissionBase::BindDelegates, );
	virtual void UnbindDelegates(UObject* TargetForDelegate) PURE_VIRTUAL(UMissionBase::UnbindDelegates, );

	FOnCompleteMissionDelegate OnCompleteMissionDelegate;

protected:

	virtual void OnConditionMet();
	virtual bool IsConditionMet() PURE_VIRTUAL(UMissionBase::IsConditionMet, return false; );

#pragma endregion

#pragma region Variables

protected:

	EMissionType MissionType;

	int32 GoalCount;
	int32 CurrentCount;

	EMissionConditionType ConditionType;

	FString MissionName;
	FString MissionDescription;

	TArray<int32> ExtraValues;

#pragma endregion

#pragma region Getters / Setters

public:

	FORCEINLINE const EMissionType& GetMissionType() const { return MissionType; }
	FORCEINLINE const int32& GetGoalCount() const { return GoalCount; }
	FORCEINLINE const int32& GetCurrentCount() const { return CurrentCount; }
	FORCEINLINE const FString& GetMissionName() const { return MissionName; }
	FORCEINLINE const FString& GetMissionDescription() const { return MissionDescription; }
	
	virtual const uint8 GetMissionIndex() const PURE_VIRTUAL(UMissionBase::GetMissionIndex, static uint8 Dummy = 0; return Dummy;);

#pragma endregion

};
