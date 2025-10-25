#include "Shops/ShopItemEntryData.h"

#include "Shops/ShopWidgets/ShopItemSlotWidget.h"

void UShopItemEntryData::Init(int32 NewSlotIndex, UTexture2D* NewItemImage, const FString& NewToolTipText, bool bShouldLock)
{
	Init(NewSlotIndex, NewItemImage, NewToolTipText);
	bIsLocked = bShouldLock;
}

void UShopItemEntryData::Init(int32 NewSlotIndex, UTexture2D* NewItemImage, const FString& NewToolTipText)
{
	SlotIndex = NewSlotIndex;
	ItemImage = NewItemImage;
	ToolTipText = NewToolTipText;
}

UTexture2D* UShopItemEntryData::GetItemImage() const
{
	return ItemImage;
}

const FString& UShopItemEntryData::GetToolTipText() const
{
	return ToolTipText;
}

bool UShopItemEntryData::IsLocked() const
{
	return bIsLocked;
}

int32 UShopItemEntryData::GetSlotIndex() const
{
	return SlotIndex;
}

void UShopItemEntryData::SetSlotIndex(int32 NewSlotIndex)
{
	SlotIndex = NewSlotIndex;
}

void UShopItemEntryData::SetLockState(bool bShouldLock)
{
	bIsLocked = bShouldLock;
}

