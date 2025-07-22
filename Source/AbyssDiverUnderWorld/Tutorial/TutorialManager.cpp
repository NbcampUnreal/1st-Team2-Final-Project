#include "TutorialManager.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "UI/TutorialSubtitle.h" // 자막 위젯 헤더 포함
#include "Blueprint/UserWidget.h"

ATutorialManager::ATutorialManager()
{
    PrimaryActorTick.bCanEverTick = false;
    mCurrentStepIndex = 0;
}

void ATutorialManager::BeginPlay()
{
    Super::BeginPlay();

    // 자막 위젯 생성
    if (mTutorialSubtitleClass)
    {
        mSubtitleWidget = CreateWidget<UTutorialSubtitle>(GetWorld(), mTutorialSubtitleClass);
        if (mSubtitleWidget)
        {
            mSubtitleWidget->AddToViewport();
            mSubtitleWidget->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (mTutorialDataTable)
    {
        mStepRowNames = mTutorialDataTable->GetRowNames();
        
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("TutorialDataTable is not assigned."));
    }

    if (mKeyboardHintPanelClass)
    {
        mKeyboardHintPanel = CreateWidget<UKeyboardHintPanel>(GetWorld(), mKeyboardHintPanelClass);
        if (mKeyboardHintPanel)
        {
            mKeyboardHintPanel->AddToViewport();
            mKeyboardHintPanel->SetVisibility(ESlateVisibility::Hidden); // 기본은 숨김
        }
    }
}

void ATutorialManager::PlayCurrentStep()
{
    if (!mTutorialDataTable || !mStepRowNames.IsValidIndex(mCurrentStepIndex))
        return;

    FName rowName = mStepRowNames[mCurrentStepIndex];
    const FTutorialStepData* stepData = mTutorialDataTable->FindRow<FTutorialStepData>(rowName, TEXT("TutorialManager"));

    if (stepData)
    {
        // 자막 출력
        if (mSubtitleWidget)
        {
            mSubtitleWidget->SetSubtitleText(stepData->SubtitleText);
            mSubtitleWidget->SetVisibility(ESlateVisibility::Visible);
        }

        //키보드 힌트 패널 제어
        if (mKeyboardHintPanel)
        {
            mKeyboardHintPanel->SetHintByKey(stepData->HintKey); // 힌트 내용은 항상 갱신

            // 🔸 Move 상태일 때만 보여주고, 나머지는 숨김
            if (stepData->HintKey == ETutorialHintKey::Move)
            {
                mKeyboardHintPanel->SetVisibility(ESlateVisibility::Visible);
            }
            else
            {
                mKeyboardHintPanel->SetVisibility(ESlateVisibility::Hidden);
            }
        }

        // 자동 진행이면 타이머
        if (!stepData->bWaitForPlayerTrigger)
        {
            GetWorldTimerManager().SetTimer(
                mStepTimerHandle,
                this,
                &ATutorialManager::AdvanceStep,
                stepData->DisplayDuration,
                false
            );
        }
    }
}


void ATutorialManager::AdvanceStep()
{
    mCurrentStepIndex++;
    PlayCurrentStep();
}

void ATutorialManager::TriggerNextStep()
{
    if (mStepTimerHandle.IsValid())
    {
        GetWorldTimerManager().ClearTimer(mStepTimerHandle);
    }

    AdvanceStep();
}
