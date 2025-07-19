#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TutorialStepData.generated.h"

USTRUCT(BlueprintType)
struct FTutorialStepData : public FTableRowBase
{
    GENERATED_BODY()

    // 자막 출력 텍스트
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText SubtitleText;

    // 몇 초 동안 보여줄지
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DisplayDuration = 5.0f;

    // 자동 전환이 아니라 플레이어 행동을 기다리는 단계인지
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bWaitForPlayerTrigger = false;
};
