#pragma once

#include "CoreMinimal.h"
#include "Missions/MissionBase.h"

#include "KillMonsterMission.generated.h"

enum class EKillMonsterMission : uint8;
enum class EUnitId : uint8;

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
		const EUnitId& InUnitId
	)
		: FMissionInitParams(InMissionType, InGoalCount, InConditionType, InMissionName, InMissionDescription, InExtraValues)
	{
		UnitId = InUnitId;
	}

	EUnitId UnitId;

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

	virtual bool IsConditionMet() override;

	virtual void OnDamaged(AActor* DamagedActor, AActor* DamageCauser, const float& ReceivedDamage);
	virtual void OnDead(AActor* DamageCauser, AActor* DeadActor);

#pragma endregion

#pragma region Variables

protected:

	EKillMonsterMission MissionIndex;
	EUnitId UnitId;

#pragma endregion


#pragma region Getters / Setters

public:

	virtual const uint8 GetMissionIndex() const override;

#pragma endregion
};
