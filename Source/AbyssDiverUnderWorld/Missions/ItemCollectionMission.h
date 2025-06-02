#pragma once

#include "CoreMinimal.h"
#include "Missions/MissionBase.h"

#include "ItemCollectionMission.generated.h"

enum class EItemCollectMission : uint8;

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

	virtual void InitMission(const FMissionInitParams& Params, const uint8& NewMissionIndex) override;
	void InitMission(const FMissionInitParams& Params, const EItemCollectMission& NewMissionIndex);

	virtual void BindDelegates(UObject* TargetForDelegate) override;
	virtual void UnbindDelegates(UObject* TargetForDelegate) override;

protected:

	virtual bool IsConditionMet() override;
	virtual void OnConditionMet() override;

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
