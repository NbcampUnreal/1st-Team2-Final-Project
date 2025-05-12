// Fill out your copyright notice in the Description page of Project Settings.
#include "Inventory/ADInventoryComponent.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "DataRow/FADItemDataRow.h"
#include "UI/AllInventoryWidget.h"
#include <Net/UnrealNetwork.h>
#include "AbyssDiverUnderWorld.h"
#include "ADInventoryComponent.h"
#include <Kismet/KismetMathLibrary.h>
#include "DrawDebugHelpers.h"

UADInventoryComponent::UADInventoryComponent() :
	ItemDataTable(nullptr),
	InventoryWidgetClass(nullptr),
	TotalWeight(0),
	TotalPrice(0),
	WeightMax(100),
	bInventoryWidgetShowed(false), 
	InventoryWidgetInstance(nullptr)
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	InventorySizeByType = { 3, 2, 9, 1, 3 };

	//ConstructorHelpers::FClassFinder<UAllInventoryWidget> AllInventoryWidget(TEXT("");)

	for (int32 i = 0; i < static_cast<int32>(EItemType::Max); ++i)
	{
		InventoryIndexMapByType.FindOrAdd(static_cast<EItemType>(i));
		InventoryIndexMapByType[static_cast<EItemType>(i)].Init(-1, InventorySizeByType[i]);
	}

}

void UADInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (InventoryWidgetClass && PC && PC->IsLocalController())
	{
		InventoryWidgetInstance = CreateWidget<UAllInventoryWidget>(PC, InventoryWidgetClass);
		LOG(TEXT("WidgetCreate!"));

		if (InventoryWidgetInstance)
		{
			InventoryWidgetInstance->AddToViewport();
			InventoryWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		}
		InventoryWidgetInstance->InitializeInventoriesInfo(this);
	}

	if (ItemDataTable)
	{
		const TMap<FName, uint8*>& RowMap = ItemDataTable->GetRowMap();
		for (const TPair<FName, uint8*>& Pair : RowMap)
		{
			FFADItemDataRow* FoundRow = ItemDataTable->FindRow<FFADItemDataRow>(Pair.Key, TEXT("LookupItem"));

			FItemData NewItem = { FoundRow->Name, FoundRow->Id, FoundRow->Quantity, FoundRow->Amount, FoundRow->Weight, FoundRow->Price, FoundRow->ItemType, FoundRow->Thumbnail };
			if (NewItem.ItemType == EItemType::Exchangable)
			{
				NewItem.Mass = 0;
				NewItem.Price = 0;
			}
			if (InventoryIndexMapByType.Contains(FoundRow->ItemType) && GetTypeInventoryEmptyIndex(FoundRow->ItemType) != -1)
			{
				InventoryList.AddItem(NewItem);
			}
		}
	}
}

void UADInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UADInventoryComponent, InventoryList);
}

void UADInventoryComponent::S_DropItem_Implementation(FItemData ItemData)
{
	for (int8 i = 0; i < ItemData.Quantity; ++i)
	{
		FFADItemDataRow* FoundRow = ItemDataTable->FindRow<FFADItemDataRow>(ItemData.Name, TEXT("LookupItem")); //*데이터 테이블 위치 확정되면 수정
		if(FoundRow->SpawnActor)
			GetWorld()->SpawnActor<AActor>(FoundRow->SpawnActor, GetDropLocation(), FRotator::ZeroRotator);
	}
}

void UADInventoryComponent::AddInventoryItem(FItemData ItemData)
{
	if (TotalWeight + ItemData.Mass <= WeightMax)
	{
		FFADItemDataRow* FoundRow = ItemDataTable->FindRow<FFADItemDataRow>(ItemData.Name, TEXT("LookupItem")); //*데이터 테이블 위치 확정되면 수정
		if (FoundRow)
		{
			int16 ItemIndex = FindItemIndexById(ItemData.Name);
			if (ItemIndex > -1)
			{
				FItemData& Item = InventoryList.Items[ItemIndex];
				if (InventoryIndexMapByType.Contains(ItemData.ItemType) && GetTypeInventoryEmptyIndex(ItemData.ItemType) != -1 && Item.Quantity == 0)
				{
					InventoryIndexMapByType[ItemData.ItemType][GetTypeInventoryEmptyIndex(ItemData.ItemType)] = ItemIndex;
				}
				if (FoundRow->Stackable)
				{
					Item.Quantity += ItemData.Quantity;
					if (Item.ItemType == EItemType::Exchangable)
					{
						Item.Mass += ItemData.Mass;
						Item.Price += ItemData.Price;
						OnInventoryInfoUpdate(ItemData.Mass, ItemData.Price);
					}
				}
				else
				{
					if (Item.Quantity == 0)
					{
						++Item.Quantity;
					}
				}
				InventoryList.MarkItemDirty(Item); //FastArray 관련 함수
				UE_LOG(LogTemp, Warning, TEXT("Add Item, ItemIndex : %s"), *ItemData.Name.ToString());
				InventoryUIUpdate();
			}
			else
			{
				LOG(TEXT("It's not our game item."));
			}
		}
	}
}

bool UADInventoryComponent::RemoveInventoryItem(uint8 InventoryIndex, int8 Count, bool bIsDropAction)
{
	if (InventoryList.Items.IsValidIndex(InventoryIndex))
	{
		FItemData& Item = InventoryList.Items[InventoryIndex];
		if (Count != -1 && Item.Quantity < Count) return false;
		if (Count == -1)
		{
			if (bIsDropAction)
			{
				S_DropItem(Item);
			}
			Item.Quantity = 0;
			if(Item.ItemType == EItemType::Exchangable)
			{
				OnInventoryInfoUpdate(-Item.Mass, -Item.Price);
				Item.Mass = 0;
				Item.Price = 0;
			}
		}
		else 
		{
			if(Item.ItemType != EItemType::Exchangable)
				Item.Quantity -= Count;
		}
		InventoryList.MarkItemDirty(Item);
		if (Item.Quantity <= 0)
		{
			for (int i = 0; i < InventoryIndexMapByType[InventoryList.Items[InventoryIndex].ItemType].Num(); ++i)
			{
				if (InventoryIndexMapByType[Item.ItemType][i] == InventoryIndex)
					InventoryIndexMapByType[Item.ItemType][i] = -1;
			}
		}
		InventoryUIUpdate();
		
		return true;
	}
	else
	{
		LOG(TEXT("Invalid Inventory Index"));
		return false;
	}
}

void UADInventoryComponent::TransferSlots(uint8 FromIndex, uint8 ToIndex)
{
	if (InventoryList.Items.IsValidIndex(FromIndex) && InventoryList.Items.IsValidIndex(ToIndex))
	{
		FItemData& TempItem = InventoryList.Items[FromIndex];
		EItemType Type = TempItem.ItemType;
		for (int i = 0; i < InventoryIndexMapByType[Type].Num(); ++i)
		{
			if (InventoryIndexMapByType[Type][i] == ToIndex)
				InventoryIndexMapByType[Type][i] = FromIndex;
			else if(InventoryIndexMapByType[Type][i] == FromIndex)
				InventoryIndexMapByType[Type][i] = ToIndex;
		}
		InventoryUIUpdate();
	}
	else
	{
		LOG(TEXT("Invalid Inventory Index"));
	}
}

void UADInventoryComponent::OnInventoryInfoUpdate(int32 MassInfo, int32 PriceInfo)
{
	TotalWeight += MassInfo;
	TotalPrice += PriceInfo;
	PrintLogInventoryData();
}

void UADInventoryComponent::InventoryUIUpdate()
{
	if (InventoryWidgetInstance == nullptr) return;
	if (InventoryUpdateDelegate.IsBound())
	{
		InventoryInfoUpdateDelegate.Broadcast(TotalWeight, TotalPrice);
	}
	if (InventoryUpdateDelegate.IsBound())
	{
		InventoryUpdateDelegate.Broadcast();
	}
}

FVector UADInventoryComponent::GetDropLocation()
{
	APawn* OwnerPawn = Cast<APlayerController>(GetOwner())->GetPawn();
	FVector CameraForward = Cast<APlayerController>(GetOwner())->PlayerCameraManager->GetCameraRotation().Vector();
	FVector DropLocation = OwnerPawn->GetActorLocation() + UKismetMathLibrary::RandomUnitVectorInConeInDegrees(CameraForward, 30) * 350.0;
	return DropLocation;
}

void UADInventoryComponent::ToggleInventoryShowed()
{
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC) return;
	if (bInventoryWidgetShowed)
	{
		bInventoryWidgetShowed = false;
		if (InventoryWidgetInstance)
		{
			InventoryWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);

			PC->bShowMouseCursor = false;
			PC->SetIgnoreLookInput(false);
			PC->SetInputMode(FInputModeGameOnly());
		}
	}
	else
	{
		bInventoryWidgetShowed = true;
		if (InventoryWidgetInstance)
		{
			InventoryWidgetInstance->SetVisibility(ESlateVisibility::Visible);
			InventoryUIUpdate();
			PC->bShowMouseCursor = true;

			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(InventoryWidgetInstance->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			InputMode.SetHideCursorDuringCapture(false);
			PC->SetIgnoreLookInput(true);
			PC->SetInputMode(InputMode);
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







