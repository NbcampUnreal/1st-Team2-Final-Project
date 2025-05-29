#pragma once

#include "MissionBaseRow.h"

#include "InteractionMissionRow.generated.h"

UENUM(BlueprintType)
enum class EInteractionMission : uint8
{
	InteractionMission1
};

USTRUCT(BlueprintType)
struct FInteractionMissionRow : public FMissionBaseRow
{
	GENERATED_BODY()

	FInteractionMissionRow()
	{
		MissionType = EMissionType::Interaction;
	}
	
	UPROPERTY(EditDefaultsOnly, Category = "InteractionMission")
	EInteractionMission Mission;
};
