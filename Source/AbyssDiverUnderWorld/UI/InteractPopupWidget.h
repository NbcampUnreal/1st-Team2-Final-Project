// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractPopupWidget.generated.h"

DECLARE_DELEGATE(FOnPopupConfirmed);
DECLARE_DELEGATE(FOnPopupCanceled);

UCLASS()
class ABYSSDIVERUNDERWORLD_API UInteractPopupWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	FOnPopupConfirmed OnPopupConfirmed;
	FOnPopupCanceled OnPopupCanceled;

	UFUNCTION(BlueprintCallable, Category = "InteractPopup")
	void ConfirmClicked();

	UFUNCTION(BlueprintCallable, Category = "InteractPopup")
	void CancelClicked();
	
};
