#pragma once

#include "CoreMinimal.h"
#include "Missions/MissionBase.h"

#include "ItemUseMission.generated.h"

enum class EItemUseMission : uint8;

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UItemUseMission : public UMissionBase
{
	GENERATED_BODY()
	
public:

	UItemUseMission();

#pragma region Methods

public:

	virtual void InitMission(const FMissionInitParams& Params, const uint8& NewMissionIndex) override;
	void InitMission(const FMissionInitParams& Params, const EItemUseMission& NewMissionIndex);

	virtual void BindDelegates(UObject* TargetForDelegate) override;
	virtual void UnbindDelegates(UObject* TargetForDelegate) override;

protected:

	virtual bool IsConditionMet() override;
	virtual void OnConditionMet() override;

#pragma endregion

#pragma region Variables

protected:

	EItemUseMission MissionIndex;

#pragma endregion


#pragma region Getters / Setters

public:

	virtual const uint8 GetMissionIndex() const override;

#pragma endregion
};
