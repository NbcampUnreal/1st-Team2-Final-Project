// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Tutorial/TutorialEnums.h"
#include "ADTutorialGameMode.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADTutorialGameMode : public AGameMode
{
    GENERATED_BODY()

public:
    AADTutorialGameMode();

    virtual void StartPlay() override;
    void AdvanceTutorialPhase();

protected:
    void HandleCurrentPhase();

    void HandlePhase_Movement();
    void HandlePhase_Sprint();
    void HandlePhase_Radar();
    void HandlePhase_Collecting();
    void HandlePhase_Drone();
    void HandlePhase_LightToggle();
    void HandlePhase_Items();
    void HandlePhase_OxygenWarning();
    void HandlePhase_Revival();
    void HandlePhase_Complete();

    UFUNCTION(BlueprintCallable)
    void SpawnDownedNPC();

    UPROPERTY()
    TObjectPtr<class AADPlayerController> TutorialPlayerController;
};
