#pragma once

#include "CoreMinimal.h"
#include "Missions/MissionBase.h"

#include "KillMonsterMission.generated.h"

enum class EKillMonsterMission : uint8;
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

	virtual void InitMission(const FMissionInitParams& Params, const uint8& NewMissionIndex) override;
	void InitMission(const FMissionInitParams& Params, const EKillMonsterMission& NewMissionIndex);

	virtual void BindDelegates(UObject* TargetForDelegate) override;

protected:

	virtual void OnConditionMet() override;

#pragma endregion

#pragma region Variables

protected:

	EKillMonsterMission MissionIndex;

#pragma endregion


#pragma region Getters / Setters

public:

	virtual const uint8 GetMissionIndex() const override;

#pragma endregion
};
