#include "Shops/ShopBuyListEntryData.h"

int32 UShopBuyListEntryData::GetItemCount()
{
	return ItemCount;
}

void UShopBuyListEntryData::SetItemCount(int32 NewItemCount)
{
	ItemCount = NewItemCount;
}
