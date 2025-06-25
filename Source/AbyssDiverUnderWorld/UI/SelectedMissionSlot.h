// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SelectedMissionSlot.generated.h"

class UTextBlock;
class UImage;

UCLASS()
class ABYSSDIVERUNDERWORLD_API USelectedMissionSlot : public UUserWidget
{
	GENERATED_BODY()
#pragma region Method
public:
	void OnMissionFinished();
	void SetMissionTitle(const FString& Title);
#pragma endregion

#pragma region Variable
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> MissionBG;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> FinishLineImage;
#pragma endregion
};
