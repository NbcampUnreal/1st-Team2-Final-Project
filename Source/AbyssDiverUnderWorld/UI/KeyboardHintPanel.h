// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Tutorial/TutorialStepData.h"
#include "Components/WidgetSwitcher.h"
#include "KeyboardHintPanel.generated.h"

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UKeyboardHintPanel : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void SetHintByKey(ETutorialHintKey HintKey);

protected:
    virtual void NativeOnInitialized() override;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UWidgetSwitcher> Switcher_KeyboardHints;
	
};
