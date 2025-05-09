// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InventorySlotWidget.h"
#include "Components/RichTextBlock.h"
#include "Container/FStructContainer.h"
#include "Components/Image.h"

void UInventorySlotWidget::SetItemData(FItemData ItemInfo, int32 Index)
{
	if (!QuantityText && !Image)
		return;
	QuantityText->SetText(FText::FromString(FString::Printf(TEXT("%d"), ItemInfo.Quantity)));
	if (ItemInfo.Quantity == 0)
	{
		QuantityText->SetVisibility(ESlateVisibility::Hidden);
	}
	Image->SetBrushFromTexture(ItemInfo.Thumbnail);
}
