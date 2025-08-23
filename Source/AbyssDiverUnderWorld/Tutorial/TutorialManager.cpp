#include "TutorialManager.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Tutorial/TutorialEnums.h"
#include "TutorialStepData.h"
#include "Kismet/GameplayStatics.h"
#include "UI/TutorialSubtitle.h" 
#include "UI/TutorialHintPanel.h"
#include "UI/TutorialHighlighting.h"
#include "Framework/ADTutorialGameState.h"
#include "Framework/ADTutorialPlayerController.h"
#include "Framework/ADTutorialGameMode.h"
#include "Blueprint/UserWidget.h"

ATutorialManager::ATutorialManager()
{
	PrimaryActorTick.bCanEverTick = true;
	CurrentStepIndex = 0;

	bIsPlayerHoldingKey = false;
	bIsGaugeObjectiveActive = false;
	DisplayGaugeValue = 0.f;
}

void ATutorialManager::BeginPlay()
{
	Super::BeginPlay();

	CachedGameMode = GetWorld()->GetAuthGameMode<AADTutorialGameMode>();
	if (!CachedGameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("ATutorialManager: AADTutorialGameMode not found!"));
	}

	if (TutorialSubtitleClass)
	{
		SubtitleWidget = CreateWidget<UTutorialSubtitle>(GetWorld(), TutorialSubtitleClass);
		if (SubtitleWidget)
		{
			SubtitleWidget->AddToViewport(-100);
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
			TutorialHintPanel->AddToViewport(-50);
			TutorialHintPanel->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (GaugeWidgetClass)
	{
		GaugeWidget = CreateWidget<UUserWidget>(GetWorld(), GaugeWidgetClass);
		if (GaugeWidget)
		{
			GaugeWidget->AddToViewport(-10);
			GaugeWidget->SetVisibility(ESlateVisibility::Collapsed);
			GaugeProgressBar = Cast<UProgressBar>(GaugeWidget->GetWidgetFromName(TEXT("GaugeBar")));
		}
	}

	if (HighlightingWidgetClass)
	{
		HighlightingWidget = CreateWidget<UTutorialHighlighting>(GetWorld(), HighlightingWidgetClass);
		if (HighlightingWidget)
		{
			HighlightingWidget->AddToViewport(-5); 
			HighlightingWidget->HighlightEnd(); 
		}
	}

	if (AADTutorialGameState* TutorialGS = GetWorld()->GetGameState<AADTutorialGameState>())
	{
		TutorialGS->OnPhaseChanged.RemoveAll(this);
		TutorialGS->OnPhaseChanged.AddDynamic(this, &ATutorialManager::OnTutorialPhaseChanged);
		OnTutorialPhaseChanged(TutorialGS->GetCurrentPhase());
	}
}

void ATutorialManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsGaugeObjectiveActive) return;

	if (CachedGameMode && !CachedGameMode->IsTypingFinishedForCurrentPhase())
	{
		return;
	}

	if ((CurrentInteractionType == EGaugeInteractionType::Hold || CurrentInteractionType == EGaugeInteractionType::Hybrid) && bIsPlayerHoldingKey)
	{
		CurrentGaugeValue += GaugeHoldValuePerSecond * DeltaTime;
		CurrentGaugeValue = FMath::Clamp(CurrentGaugeValue, 0.f, TargetGaugeValue);
	}

	if (DisplayGaugeValue != CurrentGaugeValue)
	{
		DisplayGaugeValue = FMath::FInterpTo(DisplayGaugeValue, CurrentGaugeValue, DeltaTime, GaugeInterpolationSpeed);
	}

	if (GaugeProgressBar)
	{
		GaugeProgressBar->SetPercent(DisplayGaugeValue / TargetGaugeValue);
	}

	if (CurrentGaugeValue >= TargetGaugeValue && FMath::IsNearlyEqual(DisplayGaugeValue, TargetGaugeValue, 0.01f))
	{
		bIsGaugeObjectiveActive = false;
		if (GaugeWidget)
		{
			GaugeWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		RequestAdvancePhase();
	}
}

void ATutorialManager::OnTutorialPhaseChanged(ETutorialPhase NewPhase)
{
	bIsGaugeObjectiveActive = false;
	if (GaugeWidget)
	{
		GaugeWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (HighlightingWidget)
	{
		HighlightingWidget->HighlightEnd();
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

		if (HighlightingWidget && StepDataPtr->HighlightTargetID != ETutorialHighlightTarget::None)
		{
			if (NewPhase != ETutorialPhase::Step6_Inventory)
			{
				HighlightingWidget->HighlightStart(StepDataPtr->HighlightInfo);
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
	if (CachedGameMode)
	{
		CachedGameMode->OnTypingAnimationFinished();
	}

	if (!StepData.bWaitForPlayerTrigger)
	{
		FTimerHandle WaitTimer;
		GetWorldTimerManager().SetTimer(WaitTimer, this, &ATutorialManager::RequestAdvancePhase, StepData.DisplayDuration, false);
	}
}

void ATutorialManager::RequestAdvancePhase()
{
	if (CachedGameMode)
	{
		CachedGameMode->AdvanceTutorialPhase();
	}

	else if (AADTutorialPlayerController* PC = Cast<AADTutorialPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->RequestAdvanceTutorialPhase();
	}
}

void ATutorialManager::StartGaugeObjective(EGaugeInteractionType InInteractionType, float InTargetValue, float InTapValue, float InHoldValuePerSecond)
{
	CurrentInteractionType = InInteractionType;
	TargetGaugeValue = InTargetValue;
	GaugeTapValue = InTapValue;
	GaugeHoldValuePerSecond = InHoldValuePerSecond;

	CurrentGaugeValue = 0.f;
	DisplayGaugeValue = 0.f; 
	bIsPlayerHoldingKey = false;
	bIsGaugeObjectiveActive = true;

	if (GaugeWidget && GaugeProgressBar)
	{
		GaugeProgressBar->SetPercent(0.f);
		GaugeWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void ATutorialManager::NotifyInteractionStart()
{
	if (!bIsGaugeObjectiveActive) return;

	if (CachedGameMode && !CachedGameMode->IsTypingFinishedForCurrentPhase())
	{
		return;
	}

	if (CurrentInteractionType == EGaugeInteractionType::Tap || CurrentInteractionType == EGaugeInteractionType::Hybrid)
	{
		ContributeToGaugeByTap();
	}

	if (CurrentInteractionType == EGaugeInteractionType::Hold || CurrentInteractionType == EGaugeInteractionType::Hybrid)
	{
		bIsPlayerHoldingKey = true;
	}
}

void ATutorialManager::NotifyInteractionEnd()
{
	if (CurrentInteractionType == EGaugeInteractionType::Hold || CurrentInteractionType == EGaugeInteractionType::Hybrid)
	{
		bIsPlayerHoldingKey = false;
	}
}

void ATutorialManager::ContributeToGaugeByTap()
{
	if (!bIsGaugeObjectiveActive || (CurrentInteractionType == EGaugeInteractionType::Hold)) return;
	CurrentGaugeValue += GaugeTapValue;
	CurrentGaugeValue = FMath::Clamp(CurrentGaugeValue, 0.f, TargetGaugeValue);

}

void ATutorialManager::AddGaugeProgress(float Amount)
{
	if (!bIsGaugeObjectiveActive) return;

	CurrentGaugeValue += Amount;
	CurrentGaugeValue = FMath::Clamp(CurrentGaugeValue, 0.f, TargetGaugeValue);

}


void ATutorialManager::OnInventoryInputPressed()
{
	if (AADTutorialGameState* TutorialGS = GetWorld()->GetGameState<AADTutorialGameState>())
	{
		if (TutorialGS->GetCurrentPhase() == ETutorialPhase::Step6_Inventory)
		{
			if (TutorialDataTable)
			{
				const FTutorialStepData* StepDataPtr = TutorialDataTable->FindRow<FTutorialStepData>(FName("Step6_Inventory"), TEXT(""));
				if (StepDataPtr && HighlightingWidget)
				{
					HighlightingWidget->HighlightStart(StepDataPtr->HighlightInfo);
				}
			}
		}
	}
}

void ATutorialManager::OnInventoryInputReleased()
{
	if (HighlightingWidget)
	{
		HighlightingWidget->HighlightEnd();
	}
}
