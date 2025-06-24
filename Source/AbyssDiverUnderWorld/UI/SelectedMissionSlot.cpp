// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SelectedMissionSlot.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void USelectedMissionSlot::OnMissionFinished()
{
	TitleText->SetColorAndOpacity(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f));
	MissionBG->SetColorAndOpacity(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f));
	FinishLineImage->SetVisibility(ESlateVisibility::Visible);
}

void USelectedMissionSlot::SetMissionTitle(const FString& Title)
{
	TitleText->SetText(FText::FromString(Title));
}
