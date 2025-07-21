// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/ADInGameMode.h"
#include "Tutorial/TutorialManager.h"
#include "TutorialGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API ATutorialGameMode : public AADInGameMode
{
    GENERATED_BODY()

public:
    ATutorialGameMode();

protected:
    virtual void BeginPlay() override;

    /** 레벨 내 TutorialManager 초기화 및 시작 */
    void InitializeTutorial();

public:
    /** TutorialManager 접근용 Getter */
    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    ATutorialManager* GetTutorialManager() const { return TutorialManager; }

protected:
    /** 현재 레벨에 존재하는 TutorialManager */
    UPROPERTY()
    TObjectPtr<ATutorialManager> TutorialManager;
};
