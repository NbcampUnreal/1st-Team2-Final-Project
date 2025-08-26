// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ADNameWidget.generated.h"

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UADNameWidget : public UUserWidget
{
	GENERATED_BODY()

#pragma region Method

public:
	
	/** 위젯의 현재 이름을 설정 */
	void SetNameText(const FString& NewName);
	
#pragma endregion
	
#pragma region Variable

	/** 위젯의 이름을 표시하는 RichTextBlock */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class URichTextBlock> NameTextBlock;
	
#pragma endregion
};
