#include "Shops/ShopWidgets/ShopTileView.h"

#include "Shops/ShopItemEntryData.h"

void UShopTileView::SetAllElements(const TArray<UShopItemEntryData*>& Elements)
{
	SetListItems(Elements);
}
