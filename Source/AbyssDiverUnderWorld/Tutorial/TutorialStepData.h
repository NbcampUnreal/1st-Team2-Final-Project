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

    // �ڸ� ��� �ؽ�Ʈ
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText SubtitleText;

    // �ڸ��� �� �� ���� ǥ�õ���(�ڵ� ��ȯ ��)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DisplayDuration = 5.0f;

    // �÷��̾� �Է��� ��ٷ��� �Ѿ�� �������� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 bWaitForPlayerTrigger = 0;

    // Ű���� ��Ʈ ���� �߰�
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 bShowKeyboardHint = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText KeyboardHintTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FText> KeyNames;  

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FText> KeyDescriptions; 
};
