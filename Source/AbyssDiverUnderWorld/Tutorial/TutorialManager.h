// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TutorialStepData.h"
#include "UI/TutorialHintPanel.h"
#include "TutorialManager.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API ATutorialManager : public AActor
{
	GENERATED_BODY()
	
public:
    ATutorialManager();

protected:
    virtual void BeginPlay() override;

#pragma region Method

public:
    UFUNCTION()
    void OnTutorialPhaseChanged(ETutorialPhase NewPhase);

protected:

#pragma endregion

#pragma region Variable

protected:
    UPROPERTY(EditAnywhere, Category = "Tutorial")
    TObjectPtr<UDataTable> TutorialDataTable;

    int32 CurrentStepIndex = 0;

    TArray<FName> StepRowNames;

    FTimerHandle StepTimerHandle;

    UPROPERTY(EditAnywhere, Category = "Tutorial|UI")
    TSubclassOf<class UTutorialSubtitle> TutorialSubtitleClass;

    UPROPERTY()
    TObjectPtr<UTutorialSubtitle> SubtitleWidget;

    UPROPERTY(EditAnywhere, Category = "Tutorial|UI")
    TSubclassOf<UTutorialHintPanel> TutorialHintPanelClass;

    UPROPERTY()
    TObjectPtr<UTutorialHintPanel> TutorialHintPanel;
#pragma endregion
};
