#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TutorialStepData.generated.h"

USTRUCT(BlueprintType)
struct FTutorialStepData : public FTableRowBase
{
    GENERATED_BODY()

    // �ڸ� ��� �ؽ�Ʈ
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText SubtitleText;

    // �� �� ���� ��������
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DisplayDuration = 5.0f;

    // �ڵ� ��ȯ�� �ƴ϶� �÷��̾� �ൿ�� ��ٸ��� �ܰ�����
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bWaitForPlayerTrigger = false;
};
