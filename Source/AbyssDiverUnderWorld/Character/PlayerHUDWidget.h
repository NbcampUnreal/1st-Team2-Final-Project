// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

/**
 * Player HUD를 표시하는 Widget입니다. 임시 UI를 출력하고 있습니다.
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;
	
#pragma region Method

public:

	void BindWidget(APawn* PlayerPawn);
	
	UFUNCTION(BlueprintCallable)
	void UpdateHealthText(float CurrentHealth, float MaxHealth);

	UFUNCTION(BlueprintCallable)
	void UpdateOxygenText(float CurrentOxygen, float MaxOxygen);

	UFUNCTION(BlueprintCallable)
	void UpdateStaminaText(float CurrentStamina, float MaxStamina);

#pragma endregion

#pragma region Variable

private:
	
	UPROPERTY(meta = (BindWidget))
	class URichTextBlock* HealthTextBlock;

	UPROPERTY(meta = (BindWidget))
	class URichTextBlock* OxygenTextBlock;

	UPROPERTY(meta = (BindWidget))
	class URichTextBlock* StaminaTextBlock;
	
#pragma endregion
	
};
