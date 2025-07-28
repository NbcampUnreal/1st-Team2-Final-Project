// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/TutorialSubtitle.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"

void UTutorialSubtitle::NativeConstruct()
{
    Super::NativeConstruct();

    if (Text_Subtitle)
    {
        Text_Subtitle->SetText(FText::GetEmpty());
    }
}

void UTutorialSubtitle::SetSubtitleText(const FText& NewText)
{
    GetWorld()->GetTimerManager().ClearTimer(TypingTimerHandle);

    FullText = NewText.ToString();
    CurrentCharIndex = 0;

    if (Text_Subtitle)
    {
        Text_Subtitle->SetText(FText::GetEmpty());
    }

    GetWorld()->GetTimerManager().SetTimer(
        TypingTimerHandle,
        this,
        &UTutorialSubtitle::ShowNextCharacter,
        TypingSpeed,
        true
    );
}

void UTutorialSubtitle::ShowNextCharacter()
{
    if (!Text_Subtitle) return;

    if (CurrentCharIndex < FullText.Len())
    {
        FString DisplayText = FullText.Left(CurrentCharIndex + 1);
        Text_Subtitle->SetText(FText::FromString(DisplayText));
        ++CurrentCharIndex;
    }
    else
    {
        GetWorld()->GetTimerManager().ClearTimer(TypingTimerHandle);
    }
}