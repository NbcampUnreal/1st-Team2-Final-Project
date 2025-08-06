// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NoticeWidget.generated.h"

class UTextBlock;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UNoticeWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetNoticeText(const FString& NewNotice);
	void ShowNotice();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NoticeText;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> ShowAnim;

private:
	uint8 bIsUsing : 1 = false;
	
public:
	UFUNCTION(BlueprintCallable)
	void SetbIsUsing(bool NewbIsUsing) { bIsUsing = NewbIsUsing; }
};
