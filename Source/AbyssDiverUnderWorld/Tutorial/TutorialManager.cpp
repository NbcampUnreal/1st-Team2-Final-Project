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
        }
    }

    // 튜토리얼 시작
    if (mTutorialDataTable)
    {
        mStepRowNames = mTutorialDataTable->GetRowNames();
        PlayCurrentStep();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("TutorialDataTable is not assigned."));
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

        // 추후: 여기에서 키보드 힌트 조건 체크도 가능
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
