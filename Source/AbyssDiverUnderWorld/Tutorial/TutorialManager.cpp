#include "TutorialManager.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ATutorialManager::ATutorialManager()
{
    PrimaryActorTick.bCanEverTick = false;
    mCurrentStepIndex = 0;
}

void ATutorialManager::BeginPlay()
{
    Super::BeginPlay();

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
        GEngine->AddOnScreenDebugMessage(-1, stepData->DisplayDuration, FColor::Yellow, stepData->SubtitleText.ToString());

        if (!stepData->bWaitForPlayerTrigger)
        {
            GetWorldTimerManager().SetTimer(mStepTimerHandle, this, &ATutorialManager::AdvanceStep, stepData->DisplayDuration, false);
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
