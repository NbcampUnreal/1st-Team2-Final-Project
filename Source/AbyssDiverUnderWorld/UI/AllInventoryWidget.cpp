// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AllInventoryWidget.h"
#include "InventoryWidget.h"
#include "Inventory/ADInventoryComponent.h"
#include "Components/RichTextBlock.h"

void UAllInventoryWidget::InitializeInventoriesInfo(UADInventoryComponent* InventoryComp)
{
	const TArray<int8>& InventorySizeByType = InventoryComp->GetInventorySizeByType();

	RefreshExchangableInventoryInfo(0, 0);

	if (EquipmentInventory && ConsumableInventory && ExchangableInventory)
	{
		EquipmentInventory->SetInventoryInfo(InventorySizeByType[static_cast<int8>(EItemType::Equipment)], EItemType::Equipment, InventoryComp);
		ConsumableInventory->SetInventoryInfo(InventorySizeByType[static_cast<int8>(EItemType::Consumable)], EItemType::Consumable, InventoryComp);
		ExchangableInventory->SetInventoryInfo(InventorySizeByType[static_cast<int8>(EItemType::Exchangable)], EItemType::Exchangable, InventoryComp);
	}
}

void UAllInventoryWidget::RefreshExchangableInventoryInfo(int32 Mass, int32 Price)
{
	if (MassText && PriceText)
	{
		MassText->SetText(FText::FromString(FString::Printf(TEXT("%d"), Mass)));
		PriceText->SetText(FText::FromString(FString::Printf(TEXT("%d"), Price)));
	}
}
