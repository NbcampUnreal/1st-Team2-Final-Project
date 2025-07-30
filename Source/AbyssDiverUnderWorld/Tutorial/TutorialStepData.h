#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Tutorial/TutorialEnums.h"
#include "TutorialStepData.generated.h"


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

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName HighlightTargetID;
};
