// Fill out your copyright notice in the Description page of Project Settings.
#include "Inventory/ADInventoryComponent.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "DataRow/FADItemDataRow.h"
#include "UI/AllInventoryWidget.h"
#include <Net/UnrealNetwork.h>

UADInventoryComponent::UADInventoryComponent() :
	ItemDataTable(nullptr),
	InventoryWidgetClass(nullptr),
	TotalWeight(0),
	WeightMax(100),
	bInventoryWidgetShowed(false), 
	InventoryWidgetInstance(nullptr)
{
	PrimaryComponentTick.bCanEverTick = true;

	InventorySizeByType = { 3, 2, 9, 1, 3 };

	for (int32 i = 0; i < static_cast<int32>(EItemType::Max); ++i)
	{
		InventoryIndexMapByType.FindOrAdd(static_cast<EItemType>(i));
		InventoryIndexMapByType[static_cast<EItemType>(i)].Init(-1, InventorySizeByType[i]);
	}
}

void UADInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	if (InventoryWidgetClass)
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		InventoryWidgetInstance = CreateWidget<UAllInventoryWidget>(PC, InventoryWidgetClass);

		if (InventoryWidgetInstance)
		{
			InventoryWidgetInstance->AddToViewport();
			InventoryWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
		}
		InventoryWidgetInstance->InitializeInventoriesInfo(this);
	}
}

void UADInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UADInventoryComponent, InventoryList);
}

void UADInventoryComponent::AddInventoryItem(FItemData ItemData, uint8 Count)
{
	bool bOverWeight = TotalWeight >= WeightMax;
	if (ItemDataTable && !bOverWeight)
	{
		FFADItemDataRow* FoundRow = ItemDataTable->FindRow<FFADItemDataRow>(ItemData.Name, TEXT("LookupItem")); //*데이터 테이블 위치 확정되면 수정
		if (FoundRow)
		{
			int16 ItemIndex = FindItemIndexById(ItemData.Name);
			if (ItemIndex > -1 && FoundRow->Stackable)
			{
				FItemData& Item = InventoryList.Items[ItemIndex];
				Item.Quantity += Count;
				InventoryList.MarkItemDirty(Item); //FastArray 관련 함수
			}
			else
			{
				FItemData NewItem = { ItemData.Name, ItemData.Id, Count, ItemData.Amount, ItemData.ItemType, ItemData.Thumbnail };
				if (InventoryIndexMapByType.Contains(NewItem.ItemType) && GetTypeInventoryEmptyIndex(NewItem.ItemType) != -1)
				{
					InventoryIndexMapByType[NewItem.ItemType][GetTypeInventoryEmptyIndex(NewItem.ItemType)] = InventoryList.Items.Num();
					InventoryList.AddItem(NewItem);
				}
			}
			UE_LOG(LogTemp, Warning, TEXT("Add Item, ItemIndex : %s"), *ItemData.Name.ToString());


			TotalWeight += ItemData.Mass;
			if (InventoryUpdateDelegate.IsBound())
			{
				InventoryUpdateDelegate.Broadcast(ItemData.ItemType);
			}
			PrintLogInventoryData();
		}
	}
}

void UADInventoryComponent::ToggleInventoryShowed()
{
	if (bInventoryWidgetShowed)
	{
		bInventoryWidgetShowed = false;
		if (InventoryWidgetInstance)
		{
			InventoryWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	else
	{
		bInventoryWidgetShowed = true;
		if (InventoryWidgetInstance)
		{
			InventoryWidgetInstance->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

int8 UADInventoryComponent::GetTypeInventoryEmptyIndex(EItemType ItemType)
{
	for (int i = 0; i < InventoryIndexMapByType[ItemType].Num(); ++i)
	{
		if (InventoryIndexMapByType[ItemType][i] == -1)
			return i;
	}
	return -1;
}

int16 UADInventoryComponent::FindItemIndexById(FName ItemID)
{
	for (int i = 0; i < InventoryList.Items.Num(); ++i)
	{
		if (InventoryList.Items[i].Name == ItemID)
		{
			return i;
		}
	}
	return -1;
}

void UADInventoryComponent::PrintLogInventoryData()
{
	for (FItemData& Item : InventoryList.Items)
	{
		UE_LOG(LogTemp, Warning, TEXT("Name : %s || Quntity : %d || Amount : %d"), *Item.Name.ToString(), Item.Quantity, Item.Amount);
	}
	UE_LOG(LogTemp, Warning, TEXT("InventoryWeight : %d-------------------------------------"), TotalWeight);

	for (auto& Pair : InventoryIndexMapByType)
	{
		UE_LOG(LogTemp, Warning, TEXT("ItemType : %d ############################################"), (int32)Pair.Key);
		for (int8 Index : Pair.Value)
		{
			if (Index >= 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("Index : %d, Item : %s"), Index, *InventoryList.Items[Index].Name.ToString());
			}
			else
				UE_LOG(LogTemp, Warning, TEXT("NoItems"));
		}
	}
}







