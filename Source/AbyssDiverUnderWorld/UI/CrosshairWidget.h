// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CrosshairWidget.generated.h"

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;
	
#pragma region Method

#pragma endregion
	
#pragma region Variable

protected:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> CrosshairImage;
	
#pragma endregion

#pragma region Getter Setter

public:

	UImage* GetCrosshairImage() const { return CrosshairImage; }
	
#pragma endregion
	
};
