#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "MissionBase.generated.h"

enum class EMissionType : uint8;
enum class EMissionConditionType : uint8;

DECLARE_DELEGATE_TwoParams(FOnCompleteMissionDelegate, EMissionType, uint8 /*MissionIndex*/);
DECLARE_DELEGATE_FourParams(FOnMissionProgress, EMissionType, uint8 /*index*/, int32 /*current*/, int32 /* Goal*/);

struct FMissionInitParams
{
	FMissionInitParams(
		const EMissionType InMissionType,
		const int32 InGoalCount,
		const EMissionConditionType InConditionType,
		const FString InMissionName,
		const FString InMissionDescription,
		const TArray<int32> InExtraValues,
		bool bInCompleteInstantly
	)
		: MissionType(InMissionType)
		, GoalCount(InGoalCount)
		, ConditionType(InConditionType)
		, MissionName(MoveTemp(InMissionName))
		, MissionDescription(MoveTemp(InMissionDescription))
		, ExtraValues(InExtraValues)
		, bCompleteInstantly(bInCompleteInstantly)
	{
	}

	EMissionType MissionType;
	int32 GoalCount;
	EMissionConditionType ConditionType;
	FString MissionName;
	FString MissionDescription;
	TArray<int32> ExtraValues;
	uint8 bCompleteInstantly : 1;
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

	void AddProgress(int32 Delta);

	void CompleteMission();

	FOnCompleteMissionDelegate OnCompleteMissionDelegate;
	FOnMissionProgress OnMissionProgressDelegate;
protected:
	// 파생에서 조건 평가가 필요한 경우만 사용
	virtual bool IsConditionMet() PURE_VIRTUAL(UMissionBase::IsConditionMet, return false; );

	// 진행 변화 훅
	virtual void OnMissionProgressChanged(int32 Delta);

	virtual void OnMissionCompleted();

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

	uint8 bIsCompleted : 1 = 0;

#pragma endregion

#pragma region Getters / Setters

public:

	FORCEINLINE const EMissionType& GetMissionType() const { return MissionType; }
	FORCEINLINE const int32& GetGoalCount() const { return GoalCount; }
	FORCEINLINE const int32& GetCurrentCount() const { return CurrentCount; }
	FORCEINLINE const FString& GetMissionName() const { return MissionName; }
	FORCEINLINE const FString& GetMissionDescription() const { return MissionDescription; }
	FORCEINLINE bool IsCompleted() const { return bIsCompleted != 0; }


	virtual uint8 GetMissionIndex() const PURE_VIRTUAL(UMissionBase::GetMissionIndex, return 0;);

#pragma endregion

};
