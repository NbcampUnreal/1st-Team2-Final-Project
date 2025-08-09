#include "TutorialManager.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Tutorial/TutorialEnums.h"
#include "TutorialStepData.h"
#include "Kismet/GameplayStatics.h"
#include "UI/TutorialSubtitle.h" 
#include "UI/TutorialHintPanel.h"
#include "Framework/ADTutorialGameState.h"
#include "Framework/ADPlayerController.h"
#include "Framework/ADTutorialGameMode.h"
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
	if (CurrentHighlightWidget)
	{
		CurrentHighlightWidget->RemoveFromParent();
		CurrentHighlightWidget = nullptr;
	}

	if (NewPhase == ETutorialPhase::None)
	{
		if (SubtitleWidget) SubtitleWidget->SetVisibility(ESlateVisibility::Hidden);
		if (TutorialHintPanel) TutorialHintPanel->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	if (!TutorialDataTable) return;

	const FString EnumAsString = UEnum::GetValueAsString(NewPhase);
	const FName RowName = FName(*EnumAsString.RightChop(EnumAsString.Find(TEXT("::")) + 2));
	const FTutorialStepData* StepDataPtr = TutorialDataTable->FindRow<FTutorialStepData>(RowName, TEXT("TutorialManager"));

	if (StepDataPtr)
	{
		if (SubtitleWidget)
		{

			SubtitleWidget->OnTypingCompleted.Clear();
			SubtitleWidget->OnTypingCompleted.AddLambda([this, StepDataPtr]()
				{
					if (StepDataPtr)
					{
						OnTypingFinished(*StepDataPtr);
					}
				});
			SubtitleWidget->SetSubtitleText(StepDataPtr->SubtitleText);
			SubtitleWidget->SetVisibility(ESlateVisibility::Visible);
		}

		if (TutorialHintPanel)
		{
			if (StepDataPtr->HintKey != ETutorialHintKey::None)
			{
				TutorialHintPanel->SetHintByKey(StepDataPtr->HintKey);
				TutorialHintPanel->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				TutorialHintPanel->SetVisibility(ESlateVisibility::Hidden);
			}
		}

		if (StepDataPtr->HighlightTargetID != ETutorialHighlightTarget::None)
		{
			if (const TSubclassOf<UUserWidget>* WidgetClassPtr = HighlightWidgetClasses.Find(StepDataPtr->HighlightTargetID))
			{
				CurrentHighlightWidget = CreateWidget<UUserWidget>(GetWorld(), *WidgetClassPtr);
				if (CurrentHighlightWidget)
				{
					CurrentHighlightWidget->AddToViewport(5);
				}
			}
		}
	}
	else
	{
		if (SubtitleWidget) SubtitleWidget->SetVisibility(ESlateVisibility::Hidden);
		if (TutorialHintPanel) TutorialHintPanel->SetVisibility(ESlateVisibility::Hidden);
	}
}

void ATutorialManager::OnTypingFinished(const FTutorialStepData& StepData)
{
	if (AADTutorialGameMode* GM = GetWorld()->GetAuthGameMode<AADTutorialGameMode>())
	{
		GM->OnTypingAnimationFinished();
	}

	if (!StepData.bWaitForPlayerTrigger)
	{
		FTimerHandle WaitTimer;
		GetWorldTimerManager().SetTimer(WaitTimer, this, &ATutorialManager::RequestAdvancePhase, StepData.DisplayDuration, false);
	}
}

void ATutorialManager::RequestAdvancePhase()
{
    if (AADPlayerController* PC = GetWorld()->GetFirstPlayerController<AADPlayerController>())
    {
        PC->Server_RequestAdvanceTutorialPhase();
    }
}