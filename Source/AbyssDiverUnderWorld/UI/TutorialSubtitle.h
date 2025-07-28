// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TutorialSubtitle.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UTutorialSubtitle : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable)
    void SetSubtitleText(const FText& NewText);

protected:
    void ShowNextCharacter();

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UTextBlock> Text_Subtitle;

    FString FullText;
    int32 CurrentCharIndex = 0;
    FTimerHandle TypingTimerHandle;

    UPROPERTY(EditAnywhere, Category = "Typing")
    float TypingSpeed = 0.04f;
};