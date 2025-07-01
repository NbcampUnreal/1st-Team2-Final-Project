#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "BGMDataRow.generated.h"

UENUM(BlueprintType)
enum class ESFX_BGM : uint8
{
	Sound0,
	ShallowPhase1,
	ShallowPhase2,
	ShallowPhase3,
	DeepPhase1,
	DeepPhase2,
	DeepPhase3,
	BossFight,
	ShallowBackground,
	DroneTheme,
	CampRadio,
	Max UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FBGMDataRow : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly)
	ESFX_BGM SoundType;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USoundBase> Sound;
};
