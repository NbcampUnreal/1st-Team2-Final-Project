#include "TutorialManager.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "UI/TutorialSubtitle.h" 
#include "Blueprint/UserWidget.h"

ATutorialManager::ATutorialManager()
{
    PrimaryActorTick.bCanEverTick = false;
    CurrentStepIndex = 0;
}

void ATutorialManager::BeginPlay()
{
    Super::BeginPlay();
    if (TutorialSubtitleClass)
    {
        SubtitleWidget = CreateWidget<UTutorialSubtitle>(GetWorld(), TutorialSubtitleClass);
        if (SubtitleWidget)
        {
            SubtitleWidget->AddToViewport(20);
            SubtitleWidget->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (TutorialDataTable)
    {
        StepRowNames = TutorialDataTable->GetRowNames();
        
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("TutorialDataTable is not assigned."));
    }

    if (TutorialHintPanelClass)
    {
        TutorialHintPanel = CreateWidget<UTutorialHintPanel>(GetWorld(), TutorialHintPanelClass);
        if (TutorialHintPanel)
        {
            TutorialHintPanel->AddToViewport(10);
            TutorialHintPanel->SetVisibility(ESlateVisibility::Hidden); 
        }
    }
}

void ATutorialManager::PlayCurrentStep()
{
    if (!TutorialDataTable || !StepRowNames.IsValidIndex(CurrentStepIndex))
        return;

    FName RowName = StepRowNames[CurrentStepIndex];
    const FTutorialStepData* StepData = TutorialDataTable->FindRow<FTutorialStepData>(RowName, TEXT("TutorialManager"));

    if (StepData)
    {
        if (SubtitleWidget)
        {
            SubtitleWidget->SetSubtitleText(StepData->SubtitleText);
            SubtitleWidget->SetVisibility(ESlateVisibility::Visible);
        }

        if (TutorialHintPanel)
        {
            TutorialHintPanel->SetHintByKey(StepData->HintKey);

            if (StepData->HintKey == ETutorialHintKey::Move)
            {
                TutorialHintPanel->SetVisibility(ESlateVisibility::Visible);
            }
            else
            {
                TutorialHintPanel->SetVisibility(ESlateVisibility::Hidden);
            }
        }

        if (!StepData->bWaitForPlayerTrigger)
        {
            GetWorldTimerManager().SetTimer(
                StepTimerHandle,
                this,
                &ATutorialManager::AdvanceStep,
                StepData->DisplayDuration,
                false
            );
        }
    }
}


void ATutorialManager::AdvanceStep()
{
    CurrentStepIndex++;
    PlayCurrentStep();
}

void ATutorialManager::TriggerNextStep()
{
    if (StepTimerHandle.IsValid())
    {
        GetWorldTimerManager().ClearTimer(StepTimerHandle);
    }

    AdvanceStep();
}
