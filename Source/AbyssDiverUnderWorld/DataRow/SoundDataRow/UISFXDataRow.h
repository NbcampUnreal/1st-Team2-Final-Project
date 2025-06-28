#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "UISFXDataRow.generated.h"

UENUM(BlueprintType)
enum class ESFX_UI : uint8
{
	UIClicked,
	PhaseTransition,
	SubmissionSelect,
	ShopOpenClose
};

USTRUCT(BlueprintType)
struct FUISFXDataRow : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly)
	ESFX_UI SoundType;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USoundBase> Sound;
};
