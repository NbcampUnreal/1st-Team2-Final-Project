#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "SFXDataRow.generated.h"

UENUM(BlueprintType)
enum class ESFX : uint8
{
	Sound0,
	Sound1,
	Sound2,
};

USTRUCT(BlueprintType)
struct FSFXDataRow : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly)
	ESFX SoundType;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USoundBase> Sound;
};
