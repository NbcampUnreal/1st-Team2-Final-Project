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

	
	void BindWidget(class AADPlayerController* PlayerController);

protected:
	
	UFUNCTION()
	void OnTargetViewChanged(AActor* NewViewTarget);
	
#pragma endregion

#pragma region Variable

private:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class URichTextBlock> SpectateTargetNameText;
	
#pragma endregion
};
