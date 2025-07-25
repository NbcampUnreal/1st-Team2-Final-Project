// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WarningWidget.generated.h"

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UWarningWidget : public UUserWidget
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintImplementableEvent, Category = "Warning")
	void ShowWarning();

	UFUNCTION(BlueprintImplementableEvent, Category = "Warning")
	void HideWarning();

private:
	uint8 bShowWarning : 1 = false;

public:
	UFUNCTION(BlueprintCallable)
	bool GetbShowWarning() const { return bShowWarning; }
	void SetbShowWarning(bool newbShowWarning);
};
