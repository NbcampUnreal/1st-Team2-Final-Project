#include "TutorialManager.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "UI/TutorialSubtitle.h" // �ڸ� ���� ��� ����
#include "Blueprint/UserWidget.h"

ATutorialManager::ATutorialManager()
{
    PrimaryActorTick.bCanEverTick = false;
    mCurrentStepIndex = 0;
}

void ATutorialManager::BeginPlay()
{
    Super::BeginPlay();

    // �ڸ� ���� ����
    if (mTutorialSubtitleClass)
    {
        mSubtitleWidget = CreateWidget<UTutorialSubtitle>(GetWorld(), mTutorialSubtitleClass);
        if (mSubtitleWidget)
        {
            mSubtitleWidget->AddToViewport();
        }
    }

    // Ʃ�丮�� ����
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
        // �ڸ� ���
        if (mSubtitleWidget)
        {
            mSubtitleWidget->SetSubtitleText(stepData->SubtitleText);
        }

        // �ڵ� �����̸� Ÿ�̸�
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

        // ����: ���⿡�� Ű���� ��Ʈ ���� üũ�� ����
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
