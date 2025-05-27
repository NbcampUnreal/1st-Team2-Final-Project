#include "Shops/ShopWidgets/ShopTileView.h"

#include "Shops/ShopItemEntryData.h"
#include "AbyssDiverUnderWorld.h"

void UShopTileView::SetAllElements(const TArray<UShopItemEntryData*>& Elements)
{
	FString DebugText = TEXT("Tile Initialized : (");

	for (auto& E : Elements)
	{
		DebugText += FString::FromInt(E->GetSlotIndex());
		DebugText += TEXT(", ");
	}

	DebugText = DebugText.LeftChop(2);
	DebugText += TEXT(")");

	LOGV(Warning, TEXT("%s"), *DebugText);
	SetListItems(Elements);
}
