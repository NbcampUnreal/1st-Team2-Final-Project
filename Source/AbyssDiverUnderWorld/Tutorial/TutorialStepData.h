#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TutorialStepData.generated.h"

UENUM(BlueprintType)
enum class ETutorialHintKey : uint8
{
	None UMETA(DisplayName = "None"),
	Move UMETA(DisplayName = "Move"),
	Sprint UMETA(DisplayName = "Sprint"),
	Flashlight UMETA(DisplayName = "Flashlight"),
	Revive UMETA(DisplayName = "Revive"),
	Inventory UMETA(DisplayName = "Inventory")
};

USTRUCT(BlueprintType)
struct FTutorialStepData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText SubtitleText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DisplayDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    uint8 bWaitForPlayerTrigger : 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    uint8 bShowKeyboardHint : 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText KeyboardHintTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FText> KeyNames;  

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FText> KeyDescriptions; 

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETutorialHintKey HintKey = ETutorialHintKey::None;
};
