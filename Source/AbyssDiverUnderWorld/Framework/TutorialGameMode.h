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

    /** ���� �� TutorialManager �ʱ�ȭ �� ���� */
    void InitializeTutorial();

public:
    /** TutorialManager ���ٿ� Getter */
    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    ATutorialManager* GetTutorialManager() const { return TutorialManager; }

protected:
    /** ���� ������ �����ϴ� TutorialManager */
    UPROPERTY()
    TObjectPtr<ATutorialManager> TutorialManager;
};
