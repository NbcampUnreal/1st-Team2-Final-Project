// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TutorialSubtitle.generated.h"

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UTutorialSubtitle : public UUserWidget
{
    GENERATED_BODY()

    public:
    // 전체 자막 텍스트 설정 + 타이핑 시작
    UFUNCTION(BlueprintCallable)
    void SetSubtitleText(const FText& NewText);

protected:
    virtual void NativeConstruct() override;

    // 한 글자씩 출력하는 함수
    void ShowNextCharacter();

protected:
    // 실제 자막 표시할 텍스트 블록
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UTextBlock> Text_Subtitle;

    // 전체 자막 저장
    FString FullText;

    // 현재 출력할 인덱스
    int32 CurrentCharIndex = 0;

    // 타이머 핸들
    FTimerHandle TypingTimerHandle;

    // 글자 출력 간격 (초당 몇 글자)
    UPROPERTY(EditAnywhere, Category = "Typing")
    float TypingSpeed = 0.04f;  // 40ms 간격
};
