#pragma once

#include "CoreMinimal.h"
#include "Missions/MissionBase.h"

#include "KillMonsterMission.generated.h"

enum class EKillMonsterMission : uint8;
enum class EUnitId : uint8;

class AMonster;

struct FKillMissionInitParams : public FMissionInitParams
{
	FKillMissionInitParams
	(
		const EMissionType& InMissionType,
		const int32& InGoalCount,
		const EMissionConditionType& InConditionType,
		const FString& InMissionName,
		const FString& InMissionDescription,
		const TArray<int32>& InExtraValues,
		bool bInShouldCompleteInstanly,
		const EUnitId& InUnitId,
		uint8 InNeededSimultaneousKillCount,
		float InKillInterval
	)
		: FMissionInitParams(InMissionType, InGoalCount, InConditionType, InMissionName, InMissionDescription, InExtraValues, bInShouldCompleteInstanly)
	{
		UnitId = InUnitId;
		NeededSimultaneousKillCount = InNeededSimultaneousKillCount;
		KillInterval = InKillInterval;
	}

	EUnitId UnitId;
	uint8 NeededSimultaneousKillCount;
	float KillInterval;
};

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UKillMonsterMission : public UMissionBase
{
	GENERATED_BODY()
	
public:

	UKillMonsterMission();

#pragma region Methods

public:

	virtual void InitMission(const FMissionInitParams& Params) override;
	void InitMission(const FKillMissionInitParams& Params, const EKillMonsterMission& NewMissionIndex);

	virtual void BindDelegates(UObject* TargetForDelegate) override;
	virtual void UnbindDelegates(UObject* TargetForDelegate) override;

protected:

	UFUNCTION()
	virtual void OnMonsterDead(AActor* Killer, AMonster* DeadMonster);

	virtual bool IsConditionMet() override;

	virtual void OnDamaged(AActor* DamagedActor, AActor* DamageCauser, const float& ReceivedDamage);
	virtual void OnDead(AActor* DamageCauser, AActor* DeadActor);

	

#pragma endregion

#pragma region Variables

protected:

	EKillMonsterMission MissionIndex;
	EUnitId UnitId;
	uint8 NeededSimultaneousKillCount;

	float KillInterval;

	// LastKilledTime, CachedSimultaneousKillCount
	TArray<TPair<float, uint8>>  PlayerKillInfoArray;

#pragma endregion


#pragma region Getters / Setters

public:

	virtual const uint8 GetMissionIndex() const override;

private:

	int32 GetPlayerIndex(AActor* DamageCauser);
	int32 GetPlayerIndexInternal(AActor* SuspectedPlayer);

#pragma endregion
};
