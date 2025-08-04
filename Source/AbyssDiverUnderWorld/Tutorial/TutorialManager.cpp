#include "TutorialManager.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Tutorial/TutorialEnums.h"
#include "TutorialStepData.h"
#include "Kismet/GameplayStatics.h"
#include "UI/TutorialSubtitle.h" 
#include "UI/TutorialHintPanel.h"
#include "Framework/ADTutorialGameState.h"
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

    if (AADTutorialGameState* TutorialGS = GetWorld()->GetGameState<AADTutorialGameState>())
    {

        TutorialGS->OnPhaseChanged.AddDynamic(this, &ATutorialManager::OnTutorialPhaseChanged);
        OnTutorialPhaseChanged(TutorialGS->GetCurrentPhase());
    }
}

void ATutorialManager::OnTutorialPhaseChanged(ETutorialPhase NewPhase)
{
    if (!TutorialDataTable) return;

    const FString EnumAsString = UEnum::GetValueAsString(NewPhase);
    const FName RowName = FName(*EnumAsString.RightChop(EnumAsString.Find(TEXT("::")) + 2));

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
        }
    }
    else
    {
        if (SubtitleWidget) SubtitleWidget->SetVisibility(ESlateVisibility::Hidden);
        if (TutorialHintPanel) TutorialHintPanel->SetVisibility(ESlateVisibility::Hidden);
    }
}