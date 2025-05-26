#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "BGMDataRow.generated.h"

UENUM(BlueprintType)
enum class ESFX_BGM : uint8
{
	Sound0,
	Sound1,
	Sound2,
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
