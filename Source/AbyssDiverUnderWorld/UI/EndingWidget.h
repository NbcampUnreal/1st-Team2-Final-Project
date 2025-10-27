#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EndingWidget.generated.h"

class UTextBlock;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UEndingWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TypingLine1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TypingLine2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Typing Effect")
	FString FullTextLine1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Typing Effect")
	FString FullTextLine2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Typing Effect")
	float TypingSpeed = 0.04f;

	UFUNCTION(BlueprintCallable, Category = "Typing Effect")
	void StartTypingEffect();

	void TypeNext();

	virtual void NativeDestruct() override;

protected:
	FTimerHandle TypingTimerHandle;
	FString TargetText; 
	FString CurrentDisplayText; 
	int32 TargetTextIndex = 0;

	bool bIsTypingLine2 = false;
};