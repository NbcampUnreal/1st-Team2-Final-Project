// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/Item/ADUseItem.h"
#include "Framework/ADGameInstance.h"
#include "Subsystems/DataTableSubsystem.h"

void AADUseItem::SetItemInfo(FItemData& ItemInfo)
{
	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		UDataTableSubsystem* ItemDataTableSubsystem = GI->GetSubsystem<UDataTableSubsystem>();
		FFADItemDataRow* ItemRow = ItemDataTableSubsystem->GetItemData(ItemInfo.Id);
		ItemData = ItemInfo;
		ItemData.Quantity = 1;
		ItemData.Amount = ItemRow->Amount; //추후 사용한 양에 대한 적용 필요
		SkeletalMesh->SetSkeletalMesh(ItemRow->SkeletalMesh);
	}
}
