// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Tutorial/TutorialStepData.h"
#include "Components/WidgetSwitcher.h"
#include "TutorialHintPanel.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UTutorialHintPanel : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeOnInitialized() override;

    UFUNCTION(BlueprintCallable)
    void SetHintByKey(ETutorialHintKey HintKey);

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UWidgetSwitcher> Switcher_Hints;
};
