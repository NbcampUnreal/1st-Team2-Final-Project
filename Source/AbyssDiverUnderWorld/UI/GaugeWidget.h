// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "GaugeWidget.generated.h"

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UGaugeWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) 
	TObjectPtr<UProgressBar> GaugeBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetAnim), Transient) 
	TObjectPtr<UWidgetAnimation> ShowGaugeAnim;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetAnim), Transient) 
	TObjectPtr<UWidgetAnimation> PulseAnim;

public:
	UProgressBar* GetGaugeProgressBar() const { return GaugeBar; }
	UWidgetAnimation* GetShowAnimation() const { return ShowGaugeAnim; }
	UWidgetAnimation* GetPulseAnimation() const { return PulseAnim; }
};
