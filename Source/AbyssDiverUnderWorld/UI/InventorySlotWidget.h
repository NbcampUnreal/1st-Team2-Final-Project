// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventorySlotWidget.generated.h"

struct FItemData;
class URichTextBlock;
class UImage;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()

#pragma region Method
public:
	UFUNCTION(BlueprintCallable)
	void SetItemData(FItemData ItemInfo, int32 Index);
#pragma endregion

#pragma region Variable
private:
	UPROPERTY(VisibleAnyWhere)
	int8 SlotIndex;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> QuantityText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image;
#pragma endregion
};
