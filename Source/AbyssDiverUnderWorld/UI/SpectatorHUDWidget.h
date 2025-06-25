// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SpectatorHUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API USpectatorHUDWidget : public UUserWidget
{
	GENERATED_BODY()

#pragma region Method

public:

	/** 관전 대상의 이름을 업데이트한다. */
	void UpdateSpectatorTargetName(const FString& TargetName);

	/** 현재 Controller에 바인딩한다. */
	void BindWidget(class AADPlayerController* PlayerController);

protected:

	/** Target View가 변경되었을 때 호출되는 콜백 함수 */
	UFUNCTION()
	void OnTargetViewChanged(AActor* NewViewTarget);
	
#pragma endregion

#pragma region Variable

private:

	/** 관전 대상의 이름을 표시하는 RichTextBlock 위젯 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class URichTextBlock> SpectateTargetNameText;
	
#pragma endregion
};
