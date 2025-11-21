// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/TutorialHintPanel.h"


void UTutorialHintPanel::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    SetVisibility(ESlateVisibility::Hidden);
}

void UTutorialHintPanel::SetHintByKey(ETutorialHintKey HintKey)
{
	if (HintKey == ETutorialHintKey::None)
	{
		SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	int32 Index = 0; 

	switch (HintKey)
	{
	case ETutorialHintKey::Move:
		Index = 0; 
		break;

	case ETutorialHintKey::Equipment:
		Index = 1; 
		break;

	case ETutorialHintKey::Sprint:
	case ETutorialHintKey::Flashlight:
		Index = 2;
		break;

	case ETutorialHintKey::Radar:
	case ETutorialHintKey::Revive:
		Index = 3; 
		break;

	case ETutorialHintKey::Inventory:
		Index = 4; 
		break;

	case ETutorialHintKey::NightVision:
		Index = 5; 
		break;

	default:
		Index = 0;
		break;
	}

	if (Switcher_Hints)
	{
		Switcher_Hints->SetActiveWidgetIndex(Index);
	}

	SetVisibility(ESlateVisibility::Visible);
}