// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/TutorialSubtitle.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"

namespace FHangul
{
    const TCHAR Choseong[] = { L'ㄱ', L'ㄲ', L'ㄴ', L'ㄷ', L'ㄸ', L'ㄹ', L'ㅁ', L'ㅂ', L'ㅃ', L'ㅅ', L'ㅆ', L'ㅇ', L'ㅈ', L'ㅉ', L'ㅊ', L'ㅋ', L'ㅌ', L'ㅍ', L'ㅎ' };
    const TCHAR Jungseong[] = { L'ㅏ', L'ㅐ', L'ㅑ', L'ㅒ', L'ㅓ', L'ㅔ', L'ㅕ', L'ㅖ', L'ㅗ', L'ㅘ', L'ㅙ', L'ㅚ', L'ㅛ', L'ㅜ', L'ㅝ', L'ㅞ', L'ㅟ', L'ㅠ', L'ㅡ', L'ㅢ', L'ㅣ' };
    const TCHAR Jongseong[] = { L' ', L'ㄱ', L'ㄲ', L'ㄳ', L'ㄴ', L'ㄵ', L'ㄶ', L'ㄷ', L'ㄹ', L'ㄺ', L'ㄻ', L'ㄼ', L'ㄽ', L'ㄾ', L'ㄿ', L'ㅀ', L'ㅁ', L'ㅂ', L'ㅄ', L'ㅅ', L'ㅆ', L'ㅇ', L'ㅈ', L'ㅊ', L'ㅋ', L'ㅌ', L'ㅍ', L'ㅎ' };

    constexpr int32 HANGUL_START = 0xAC00;
    constexpr int32 HANGUL_END = 0xD7A3;

    bool Decompose(TCHAR Char, int32& OutCho, int32& OutJung, int32& OutJong)
    {
        if (Char < HANGUL_START || Char > HANGUL_END)
        {
            return false;
        }

        int32 Code = Char - HANGUL_START;
        OutJong = Code % 28;
        Code /= 28;
        OutJung = Code % 21;
        Code /= 21;
        OutCho = Code;
        return true;
    }

    TCHAR Compose(int32 Cho, int32 Jung, int32 Jong)
    {
        return HANGUL_START + (Cho * 21 * 28) + (Jung * 28) + Jong;
    }
}

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

    TargetText = NewText.ToString();
    CurrentDisplayText.Empty();
    TargetTextIndex = 0;
    CurrentTypingStep = 0;

    if (Text_Subtitle)
    {
        Text_Subtitle->SetText(FText::GetEmpty());
    }
    GetWorld()->GetTimerManager().SetTimer(TypingTimerHandle, this, &UTutorialSubtitle::TypeNext, TypingSpeed, true);
}

void UTutorialSubtitle::TypeNext()
{
    if (TargetTextIndex >= TargetText.Len())
    {
        GetWorld()->GetTimerManager().ClearTimer(TypingTimerHandle);

        OnTypingCompleted.Broadcast();
        return;
    }

    TCHAR CurrentChar = TargetText[TargetTextIndex];

    int32 Cho, Jung, Jong;

    if (FHangul::Decompose(CurrentChar, Cho, Jung, Jong))
    {
        FString PreviousText = CurrentDisplayText;
        if (CurrentTypingStep > 0)
        {
            PreviousText = CurrentDisplayText.LeftChop(1);
        }

        if (CurrentTypingStep == 0) 
        {
            CurrentDisplayText = PreviousText + FHangul::Choseong[Cho];
            CurrentTypingStep++;
        }
        else if (CurrentTypingStep == 1) 
        {
            CurrentDisplayText = PreviousText + FHangul::Compose(Cho, Jung, 0);
            CurrentTypingStep++;

            if (Jong == 0)
            {
                TargetTextIndex++;
                CurrentTypingStep = 0;
            }
        }
        else if (CurrentTypingStep == 2) 
        {
            CurrentDisplayText = PreviousText + FHangul::Compose(Cho, Jung, Jong);
            TargetTextIndex++;
            CurrentTypingStep = 0;
        }
    }
    else 
    {
        CurrentDisplayText += CurrentChar;
        TargetTextIndex++;
        CurrentTypingStep = 0; 
    }

    if (Text_Subtitle)
    {
        Text_Subtitle->SetText(FText::FromString(CurrentDisplayText));
    }
}