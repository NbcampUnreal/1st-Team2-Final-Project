#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "AmbientDataRow.generated.h"

UENUM(BlueprintType)
enum class ESFX_Ambient : uint8
{
	Sound0,
	Sound1,
	Sound2,
};

USTRUCT(BlueprintType)
struct FAmbientDataRow : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly)
	ESFX_Ambient SoundType;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USoundBase> Sound;
};
