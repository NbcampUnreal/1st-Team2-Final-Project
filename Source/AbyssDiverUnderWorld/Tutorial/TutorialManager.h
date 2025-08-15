#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TutorialStepData.h"
#include "TutorialEnums.h"
#include "Framework/ADTutorialGameMode.h"
#include "Components/ProgressBar.h"
#include "TutorialManager.generated.h"

class UUserWidget;
class UTutorialSubtitle;
class UTutorialHintPanel;
class UTutorialHighlighting;

UCLASS()
class ABYSSDIVERUNDERWORLD_API ATutorialManager : public AActor
{
	GENERATED_BODY()

public:
	ATutorialManager();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

#pragma region Method
public:
	UFUNCTION()
	void OnTutorialPhaseChanged(ETutorialPhase NewPhase);
	UFUNCTION()
	void OnTypingFinished(const FTutorialStepData& StepData);

	void RequestAdvancePhase();
	void StartGaugeObjective(EGaugeInteractionType InInteractionType, float InTargetValue, float InTapValue, float InHoldValuePerSecond);
	void NotifyInteractionStart();
	void NotifyInteractionEnd();
	void AddGaugeProgress(float Amount);

protected:
	void ContributeToGaugeByTap();
#pragma endregion

#pragma region Variable
protected:
	UPROPERTY(EditAnywhere, Category = "Tutorial")
	TObjectPtr<UDataTable> TutorialDataTable;

	UPROPERTY(EditAnywhere, Category = "Tutorial|UI")
	TSubclassOf<UTutorialSubtitle> TutorialSubtitleClass;

	UPROPERTY(EditAnywhere, Category = "Tutorial|UI")
	TSubclassOf<UTutorialHintPanel> TutorialHintPanelClass;

	UPROPERTY(EditAnywhere, Category = "Tutorial|UI|Highlights")
	TSubclassOf<UTutorialHighlighting> HighlightingWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Tutorial|UI")
	TSubclassOf<UUserWidget> GaugeWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Tutorial|UI")
	float GaugeInterpolationSpeed = 5.f;

	int32 CurrentStepIndex = 0;
	TArray<FName> StepRowNames;
	EGaugeInteractionType CurrentInteractionType;
	uint8 bIsPlayerHoldingKey : 1;
	float CurrentGaugeValue = 0.f;
	float DisplayGaugeValue = 0.f;
	float TargetGaugeValue = 100.f;
	float GaugeTapValue;
	float GaugeHoldValuePerSecond;
	uint8 bIsGaugeObjectiveActive : 1;

	UPROPERTY()
	TObjectPtr<UTutorialSubtitle> SubtitleWidget;

	UPROPERTY()
	TObjectPtr<UTutorialHintPanel> TutorialHintPanel;

	UPROPERTY()
	TObjectPtr<UTutorialHighlighting> HighlightingWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> GaugeWidget;

	UPROPERTY()
	TObjectPtr<UProgressBar> GaugeProgressBar;

	FTimerHandle StepTimerHandle;

	UPROPERTY()
	TObjectPtr<AADTutorialGameMode> CachedGameMode;
#pragma endregion

#pragma region Getter, Setter
public:
	bool IsGaugeObjectiveActive() const { return bIsGaugeObjectiveActive; }
#pragma endregion
};