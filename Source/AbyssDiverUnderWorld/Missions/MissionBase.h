#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "MissionBase.generated.h"

enum class EMissionType : uint8;
enum class EMissionConditionType : uint8;
enum class EUnitId : uint8;

DECLARE_DELEGATE_TwoParams(FOnCompleteMissionDelegate, EMissionType, uint8 /*MissionIndex*/);
DECLARE_DELEGATE_FourParams(FOnMissionProgress, EMissionType, uint8 /*index*/, int32 /*current*/, int32 /* Goal*/);

struct FMissionInitParams
{
	FMissionInitParams(
		const EMissionType& InMissionType,
		const FString& InMissionName,
		const FString& InMissionDescription,
		const EMissionConditionType& InConditionType,
		const int32& InGoalCount,
		const TArray<int32>& InExtraValues,
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
	virtual void BindDelegates(UObject* TargetForDelegate);
	virtual void UnbindDelegates(UObject* TargetForDelegate);

	void AddProgress(int32 Delta);

	void CompleteMission();

	virtual void NotifyMonsterKilled(const FGameplayTagContainer& UnitTags) {}
	virtual void NotifyItemCollected(const FGameplayTagContainer& ItemTags, int32 Amount) {}
	virtual void NotifyItemUsed(const FGameplayTagContainer& ItemTags, int32 Amount) {}
	virtual void NotifyAggroTriggered(const FGameplayTagContainer& SourceTags) {}
	virtual void NotifyInteracted(const FGameplayTagContainer& InteractTags) {}

	FOnCompleteMissionDelegate OnCompleteMissionDelegate;
	FOnMissionProgress OnMissionProgressDelegate;
protected:

	// ���� ��ȭ ��
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
	FORCEINLINE bool IsCompleted() const { return bIsCompleted; }


	virtual uint8 GetMissionIndex() const PURE_VIRTUAL(UMissionBase::GetMissionIndex, return 0;);

#pragma endregion

};
