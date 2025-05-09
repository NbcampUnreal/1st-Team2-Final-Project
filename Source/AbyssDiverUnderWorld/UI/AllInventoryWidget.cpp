// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AllInventoryWidget.h"
#include "InventoryWidget.h"
#include "Inventory/ADInventoryComponent.h"

void UAllInventoryWidget::InitializeInventoriesInfo(UADInventoryComponent* InventoryComp)
{
	const TArray<int8>& InventorySizeByType = InventoryComp->GetInventorySizeByType();
	if (EquipmentInventory && ConsumableInventory && ExchangableInventory)
	{
		EquipmentInventory->SetInventoryInfo(InventorySizeByType[static_cast<int8>(EItemType::Equipment)], EItemType::Equipment, InventoryComp);
		ConsumableInventory->SetInventoryInfo(InventorySizeByType[static_cast<int8>(EItemType::Consumable)], EItemType::Consumable, InventoryComp);
		ExchangableInventory->SetInventoryInfo(InventorySizeByType[static_cast<int8>(EItemType::Exchangable)], EItemType::Exchangable, InventoryComp);
	}
}