#include "Shops/ShopItemEntryData.h"

void UShopItemEntryData::Init(int32 NewPrice, UTexture2D* NewItemImage, const FString& NewToolTipText)
{
	Price = NewPrice;
	ItemImage = NewItemImage;
	ToolTipText = NewToolTipText;
}

int32 UShopItemEntryData::GetPrice() const
{
	return Price;
}

UTexture2D* UShopItemEntryData::GetItemImage() const
{
	return ItemImage;
}

const FString& UShopItemEntryData::GetToolTipText() const
{
	return ToolTipText;
}
