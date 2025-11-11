// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseWidget.generated.h"


UCLASS()
class ABYSSDIVERUNDERWORLD_API UPauseWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeOnInitialized() override;


	UFUNCTION(BlueprintCallable, Category = "Pause")
	void RequestClose();

	UFUNCTION(BlueprintCallable, Category = "Pause")
	void PlayInAnimation();

	UFUNCTION(BlueprintCallable)
	void ShowMainPanel();

	UFUNCTION(BlueprintCallable)
	void ShowOptionsPanel();

	UFUNCTION(BlueprintCallable)
	void ShowSaveLoadPanel();

protected:
	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* WidgetSwitcher;

	UPROPERTY(meta = (BindWidget))
	class UWidget* Panel_Main;

	UPROPERTY(meta = (BindWidget))
	class UWidget* Panel_Options;

	UPROPERTY(meta = (BindWidget))
	class UWidget* Panel_SaveLoad;

	UPROPERTY(BlueprintReadOnly, Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> InAnim;

	UPROPERTY(BlueprintReadOnly, Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> OutAnim;

	UFUNCTION()
	void OnOutAnumFinished();
};
