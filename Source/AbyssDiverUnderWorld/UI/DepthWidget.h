// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DataRow/MapDepthRow.h"
#include "DepthWidget.generated.h"

class UTextBlock;
class UImage;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UDepthWidget : public UUserWidget
{
	GENERATED_BODY()
	
#pragma region Method
public:
	UFUNCTION(BlueprintCallable)
	uint8 GetZoneLevel() { return ZoneLevel; };
	UFUNCTION()
	void ApplyZoneChangeToWidget(EDepthZone Old, EDepthZone New);
	UFUNCTION()
	void SetDepthText(float Depth);
	void SetZoneText(const FString& ZoneName);

	void PlayOpenCloseAnim(bool bIsOpen);
#pragma endregion

#pragma region Variable
protected:
	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	TObjectPtr<UImage> DepthBG;

	uint8 ZoneLevel = 0;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ZoneText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DepthText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ZoneColorImg;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> DepthBGSide;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> DepthOpenAnim;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> DepthCloseAnim;
#pragma endregion
public:
	float GetCloseAnimLength() const;
};
