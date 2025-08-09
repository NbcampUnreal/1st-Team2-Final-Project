#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TutorialSubtitle.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnTypingCompleted);

UCLASS()
class ABYSSDIVERUNDERWORLD_API UTutorialSubtitle : public UUserWidget
{
    GENERATED_BODY()

protected:

    virtual void NativeConstruct() override;

#pragma region Method
public:
    UFUNCTION(BlueprintCallable)
    void SetSubtitleText(const FText& NewText);

protected:
    void TypeNext();
#pragma endregion

#pragma region Variable
public:
    FOnTypingCompleted OnTypingCompleted;

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UTextBlock> Text_Subtitle;

    UPROPERTY(EditAnywhere, Category = "Typing")
    float TypingSpeed = 0.04f;

    FTimerHandle TypingTimerHandle;
    FString TargetText;
    FString CurrentDisplayText;
    int32 TargetTextIndex = 0;
    int32 CurrentTypingStep = 0;
#pragma endregion
};