// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DragPreviewWidget.generated.h"

class UImage;
class UTexture2D;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UDragPreviewWidget : public UUserWidget
{
	GENERATED_BODY()
#pragma region Method
public:
	UFUNCTION(BlueprintCallable)
	void SetPreviewInfo(UTexture2D* Thumbnail);
#pragma endregion

#pragma region Variable
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PreviewImage;
#pragma endregion
	
};
