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

    // 자막 출력 텍스트
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText SubtitleText;

    // 자막이 몇 초 동안 표시될지(자동 전환 시)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DisplayDuration = 5.0f;

    // 플레이어 입력을 기다려야 넘어가는 스텝인지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 bWaitForPlayerTrigger = 0;

    // 키보드 힌트 관련 추가
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 bShowKeyboardHint = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText KeyboardHintTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FText> KeyNames;  

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FText> KeyDescriptions; 
};
