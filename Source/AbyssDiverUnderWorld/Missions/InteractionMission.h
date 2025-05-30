#pragma once

#include "CoreMinimal.h"
#include "Missions/MissionBase.h"

#include "InteractionMission.generated.h"

enum class EInteractionMission : uint8;

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UInteractionMission : public UMissionBase
{
	GENERATED_BODY()

public:

	UInteractionMission();

#pragma region Methods
public:

	virtual void InitMission(const FMissionInitParams& Params, const uint8& NewMissionIndex) override;
	void InitMission(const FMissionInitParams& Params, const EInteractionMission& NewMissionIndex);

	virtual void BindDelegates(UObject* TargetForDelegate) override;

protected:

	virtual void OnConditionMet() override;

#pragma endregion

#pragma region Variables

protected:

	EInteractionMission MissionIndex;

#pragma endregion


#pragma region Getters / Setters

public:

	virtual const uint8 GetMissionIndex() const override;

#pragma endregion
	
};
