// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/KeyboardHintPanel.h"


void UKeyboardHintPanel::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    SetVisibility(ESlateVisibility::Hidden);
}

void UKeyboardHintPanel::SetHintByKey(ETutorialHintKey HintKey)
{
    // NoneÀÌ¸é ²¨ÁÜ
    if (HintKey == ETutorialHintKey::None)
    {
        SetVisibility(ESlateVisibility::Hidden);
        return;
    }

    int32 Index = 0;

    switch (HintKey)
    {
    case ETutorialHintKey::Move:       Index = 0; break;
    case ETutorialHintKey::Sprint:     Index = 1; break;
    case ETutorialHintKey::Flashlight: Index = 2; break;
    case ETutorialHintKey::Revive:     Index = 3; break;
    case ETutorialHintKey::Inventory:  Index = 4; break;
    default:                           Index = 0; break;
    }

    if (Switcher_KeyboardHints)
    {
        Switcher_KeyboardHints->SetActiveWidgetIndex(Index);
    }

    SetVisibility(ESlateVisibility::Visible);
}
