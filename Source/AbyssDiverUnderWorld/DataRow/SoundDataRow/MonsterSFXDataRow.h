#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "MonsterSFXDataRow.generated.h"

UENUM(BlueprintType)
enum class ESFX_Monster : uint8
{
	Sound0,
	Sound1,
	Sound2,
};

USTRUCT(BlueprintType)
struct FMonsterSFXDataRow : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly)
	ESFX_Monster SoundType;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USoundBase> Sound;
};
