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
    // ��ü �ڸ� �ؽ�Ʈ ���� + Ÿ���� ����
    UFUNCTION(BlueprintCallable)
    void SetSubtitleText(const FText& NewText);

protected:
    virtual void NativeConstruct() override;

    // �� ���ھ� ����ϴ� �Լ�
    void ShowNextCharacter();

protected:
    // ���� �ڸ� ǥ���� �ؽ�Ʈ ���
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UTextBlock> Text_Subtitle;

    // ��ü �ڸ� ����
    FString FullText;

    // ���� ����� �ε���
    int32 CurrentCharIndex = 0;

    // Ÿ�̸� �ڵ�
    FTimerHandle TypingTimerHandle;

    // ���� ��� ���� (�ʴ� �� ����)
    UPROPERTY(EditAnywhere, Category = "Typing")
    float TypingSpeed = 0.04f;  // 40ms ����
};
