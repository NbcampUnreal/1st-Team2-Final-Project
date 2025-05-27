// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ADNightVisionGoggle.generated.h"

class UProgressBar;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UADNightVisionGoggle : public UUserWidget
{
	GENERATED_BODY()
#pragma region Method
public:
	virtual void NativeConstruct() override;
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void NightVigionUse();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void NightVigionUnUse();
#pragma endregion

#pragma region Variable
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> BatteryAmount;
private:
	int32 BatteryMax;
#pragma endregion

#pragma region Setter
public:
	UFUNCTION(BlueprintCallable)
	void SetBatteryAmount(int32 Amount);
#pragma endregion

};
