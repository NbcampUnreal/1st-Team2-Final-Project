#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TutorialEnums.h"
#include "Components/ProgressBar.h"
#include "TutorialManager.generated.h"

class UUserWidget;
class UTutorialSubtitle;
class UTutorialHintPanel;
class UTutorialHighlighting;
class AADTutorialGameMode;
struct FTutorialStepData;

UCLASS()
class ABYSSDIVERUNDERWORLD_API ATutorialManager : public AActor
{
	GENERATED_BODY()

	public:
	ATutorialManager();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	void RequestAdvancePhase();

	void StartGaugeObjective(EGaugeInteractionType InInteractionType, float InTargetValue, float InTapValue, float InHoldValuePerSecond);
	void NotifyInteractionStart();
	void NotifyInteractionEnd();

	void OnInventoryInputPressed();
	void OnInventoryInputReleased();

	bool IsGaugeObjectiveActive() const { return bIsGaugeObjectiveActive; }

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void AddGaugeProgress(float Amount);
protected:
	UFUNCTION()
	void OnTutorialPhaseChanged(ETutorialPhase NewPhase);

	UFUNCTION()
	void OnTypingFinished(const FTutorialStepData& StepData);

	void ContributeToGaugeByTap();

	UFUNCTION()
	void OnSubtitleTypingCompleted();
private:
	UPROPERTY(EditAnywhere, Category = "Tutorial|Data")
	TObjectPtr<UDataTable> TutorialDataTable;

	UPROPERTY(EditAnywhere, Category = "Tutorial|UI")
	TSubclassOf<UTutorialSubtitle> TutorialSubtitleClass;

	UPROPERTY(EditAnywhere, Category = "Tutorial|UI")
	TSubclassOf<UTutorialHintPanel> TutorialHintPanelClass;

	UPROPERTY(EditAnywhere, Category = "Tutorial|UI")
	TSubclassOf<UTutorialHighlighting> HighlightingWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Tutorial|UI")
	TSubclassOf<UUserWidget> GaugeWidgetClass;

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

	UPROPERTY()
	TObjectPtr<AADTutorialGameMode> CachedGameMode;

	UPROPERTY(EditAnywhere, Category = "Tutorial|UI")
	float GaugeInterpolationSpeed = 15.f;

	EGaugeInteractionType CurrentInteractionType;
	uint8 bIsPlayerHoldingKey : 1;
	float CurrentGaugeValue = 0.f;
	float DisplayGaugeValue = 0.f;
	float TargetGaugeValue = 100.f;
	float GaugeTapValue;
	float GaugeHoldValuePerSecond;
	uint8 bIsGaugeObjectiveActive : 1;

	int32 CurrentStepIndex;
	TArray<FName> StepRowNames;

	const FTutorialStepData* CurrentStepDataPtr;
};