// Fill out your copyright notice in the Description page of Project Settings.
#include "Inventory/ADInventoryComponent.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "DataRow/FADItemDataRow.h"
#include "UI/AllInventoryWidget.h"
#include <Net/UnrealNetwork.h>
#include "AbyssDiverUnderWorld.h"
#include <Kismet/KismetMathLibrary.h>
#include "DrawDebugHelpers.h"
#include "Framework/ADGameInstance.h"
#include "Subsystems/DataTableSubsystem.h"
#include "Framework/ADPlayerState.h"
#include "Interactable/Item/ADUseItem.h"
#include "Interactable/Item/UseFunction/UseStrategy.h"
#include <Actions/PawnActionsComponent.h>
#include "GameFramework/Character.h"

UADInventoryComponent::UADInventoryComponent() :
	InventoryWidgetClass(nullptr),
	TotalWeight(0),
	TotalPrice(0),
	WeightMax(100),
	bInventoryWidgetShowed(false), 
	bCanUseItem(true),
	CurrentEquipmentIndex(-1),
	CurrentEquipmentInstance(nullptr),
	InventoryWidgetInstance(nullptr),
	ItemDataTableSubsystem(nullptr)
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	InventorySizeByType = { 3, 2, 9, 1, 3 };

	ConstructorHelpers::FClassFinder<UAllInventoryWidget> AllInventoryWidget(TEXT("/Game/_AbyssDiver/Blueprints/UI/InventoryUI/WBP_AllInventoryWidget"));
	if (AllInventoryWidget.Succeeded())
	{
		InventoryWidgetClass = AllInventoryWidget.Class;
	}	

	//ADGameInstacne 생기면 삭제 36, 37
	ConstructorHelpers::FObjectFinder<UDataTable> ItemDataTable(TEXT("/Game/_AbyssDiver/DataTable/DT_Items.DT_Items"));
	TestItemDataTable = ItemDataTable.Object;

	//if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	//{
	//	ItemDataTableSubsystem = GI->GetSubsystem<UDataTableSubsystem>();
	//}

	for (int32 i = 0; i < static_cast<int32>(EItemType::Max); ++i)
	{
		InventoryIndexMapByType.FindOrAdd(static_cast<EItemType>(i));
		InventoryIndexMapByType[static_cast<EItemType>(i)].Init(-1, InventorySizeByType[i]);
	}
}

void UADInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	
	InventoryInitialize();

}

void UADInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UADInventoryComponent, InventoryList);
	DOREPLIFETIME(UADInventoryComponent, TotalPrice);
	DOREPLIFETIME(UADInventoryComponent, TotalWeight);
	DOREPLIFETIME(UADInventoryComponent, CurrentEquipmentIndex);
	DOREPLIFETIME(UADInventoryComponent, CurrentEquipmentInstance);
}

void UADInventoryComponent::S_DropItem_Implementation(FItemData ItemData)
{
	for (int8 i = 0; i < ItemData.Quantity; ++i)
	{
		AADUseItem* SpawnItem = GetWorld()->SpawnActor<AADUseItem>(AADUseItem::StaticClass(), GetDropLocation(), FRotator::ZeroRotator);
		SpawnItem->SetItemInfo(ItemData);
	}
}

void UADInventoryComponent::Equip(FItemData ItemData, int8 Index)
{
	if (ItemData.ItemType != EItemType::Equipment || ItemData.Quantity == 0) return;

	APlayerState* PS = Cast<APlayerState>(GetOwner());
	APlayerController* PC = Cast<APlayerController>(PS->GetOwningController());
	if (!PC) return;
	APawn* Pawn = Cast<APawn>(PC->GetPawn());
	if (!Pawn) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = Pawn;
	SpawnParams.Owner = Pawn;

	AADUseItem* SpawnedItem = GetWorld()->SpawnActor<AADUseItem>(AADUseItem::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	LOG(TEXT("SpawnItem"));
	if (SpawnedItem)
	{
		ACharacter* Character = Cast<ACharacter>(Pawn);
		USkeletalMeshComponent* MeshComp = Character->GetMesh();
		if (MeshComp)
		{
			SpawnedItem->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("Hand_R"));
			SpawnedItem->SetItemInfo(ItemData);
			LOG(TEXT("EquipItem"));
			CurrentEquipmentInstance = SpawnedItem;
			SetEquipInfo(Index, SpawnedItem);
		}
	}

}

void UADInventoryComponent::UnEquip()
{
	if(CurrentEquipmentInstance)
		CurrentEquipmentInstance->Destroy();
	SetEquipInfo(-1, nullptr);
	LOG(TEXT("UnEquipItem"));
}

void UADInventoryComponent::OnUseCoolTimeEnd()
{
	bCanUseItem = true;
}

void UADInventoryComponent::SetEquipInfo(int8 TypeInventoryIndex, AADUseItem* SpawnItem)
{
	CurrentEquipmentIndex = TypeInventoryIndex;
	CurrentEquipmentInstance = SpawnItem;
}

void UADInventoryComponent::AddInventoryItem(FItemData ItemData)
{
	if (TotalWeight + ItemData.Mass <= WeightMax)
	{
		FFADItemDataRow* FoundRow = TestItemDataTable->FindRow<FFADItemDataRow>(ItemData.Name, TEXT("Lookup Item"))/*ItemDataTableSubsystem->GetItemData(ItemData.Id)*/;
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

void UADInventoryComponent::S_UseInventoryItem_Implementation(EItemType ItemType, int32 InventoryIndex)
{
	if (ItemType == EItemType::Equipment)
	{
		if (InventoryIndex > InventoryIndexMapByType[ItemType].Num() || InventoryIndexMapByType[ItemType][InventoryIndex - 1] == -1 ) return;
	}
	else if (ItemType == EItemType::Consumable)
	{
		if (InventoryList.Items[InventoryIndex].Quantity == 0 || InventoryList.Items.Num() < InventoryIndex) return;
	}

	if (!bCanUseItem) return;

	bCanUseItem = false;

	FTimerHandle UseCoolTimeHandle;
	GetWorld()->GetTimerManager().SetTimer(UseCoolTimeHandle, this, &UADInventoryComponent::OnUseCoolTimeEnd, 1.0f, false);


	FItemData& Item = ItemType == EItemType::Consumable ? InventoryList.Items[InventoryIndex] : InventoryList.Items[InventoryIndexMapByType[ItemType][InventoryIndex - 1]];
	LOG(TEXT("%d %d"), CurrentEquipmentIndex, InventoryIndex);
	if (ItemType == EItemType::Equipment)
	{
		if (CurrentEquipmentIndex == InventoryIndex-1)
		{
			UnEquip();
		}
		else
		{
			if (CurrentEquipmentIndex != -1)
			{
				UnEquip();
				LOG(TEXT("ChangeEquipment"));
			}
			Equip(Item, InventoryIndex-1);
		}
	}
	else if (ItemType == EItemType::Consumable)
	{
		FFADItemDataRow* FoundRow = TestItemDataTable->FindRow<FFADItemDataRow>(Item.Name, TEXT("Lookup Item"));
		if (!FoundRow->UseFunction) return;

		if (FoundRow && FoundRow->UseFunction)
		{
			UUseStrategy* Strategy = Cast<UUseStrategy>(NewObject<UObject>(this, FoundRow->UseFunction));
			if (Strategy)
			{
				Strategy->Use(GetOwner());
			}
		}
		RemoveInventoryItem(InventoryIndex, 1, false);
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
	//PrintLogInventoryData();
}

FVector UADInventoryComponent::GetDropLocation()
{
	APlayerController* PC = Cast<APlayerController>(Cast<AADPlayerState>(GetOwner())->GetPlayerController());
	APawn* OwnerPawn = PC->GetPawn();
	FVector CameraForward = PC->PlayerCameraManager->GetCameraRotation().Vector();
	FVector DropLocation = OwnerPawn->GetActorLocation() +FVector(0, 0, 200) + UKismetMathLibrary::RandomUnitVectorInConeInDegrees(CameraForward, 30) * 350.0;
	return DropLocation;
}

void UADInventoryComponent::ToggleInventoryShowed()
{
	APlayerController* PC = Cast<APlayerController>(Cast<AADPlayerState>(GetOwner())->GetPlayerController());
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

void UADInventoryComponent::InventoryInitialize()
{
	APlayerController* PC = Cast<APlayerController>(Cast<AADPlayerState>(GetOwner())->GetPlayerController());
	if (InventoryWidgetClass && PC && PC->IsLocalController())
	{
		InventoryWidgetInstance = CreateWidget<UAllInventoryWidget>(PC, InventoryWidgetClass);
		LOG(TEXT("WidgetCreate!"));

		if (InventoryWidgetInstance)
		{
			InventoryWidgetInstance->AddToViewport();
			InventoryWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
			InventoryWidgetInstance->InitializeInventoriesInfo(this);
		}
	}

	//ADGameInstacne 생기면 삭제 70~74
	if (TestItemDataTable)
	{
		TArray<FFADItemDataRow*> ItemAllRows;
		TestItemDataTable->GetAllRows<FFADItemDataRow>(TEXT("ItemDataTable"), ItemAllRows);

		if (ItemAllRows.Num() > 0/*ItemDataTableSubsystem*/)
		{
			int8 ItemRowNum = ItemAllRows.Num()/*ItemDataTableSubsystem->GetItemDataTableArrayNum()*/;
			for (int8 i = 0; i < ItemRowNum; ++i)
			{
				FFADItemDataRow* FoundRow = ItemAllRows[i]/*ItemDataTableSubsystem->GetItemData(i)*/;
				if (FoundRow)
				{
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
	}
}

void UADInventoryComponent::OnRep_InventoryList()
{
	RebuildIndexMap();
	InventoryUIUpdate();
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

FItemData UADInventoryComponent::CurrentEquipmentItemData()
{
	int8 Index = InventoryIndexMapByType[EItemType::Equipment][CurrentEquipmentIndex];
	return InventoryList.Items[Index];
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

void UADInventoryComponent::RebuildIndexMap()
{
	// 모든 값을 -1로 초기화
	for (auto& Pair : InventoryIndexMapByType)
	{
		for (int8& Idx : Pair.Value)
			Idx = -1;
	}

	// InventoryList를 돌면서 채우기
	for (int16 ItemIdx = 0; ItemIdx < InventoryList.Items.Num(); ++ItemIdx)
	{
		const FItemData& Item = InventoryList.Items[ItemIdx];
		if (Item.Quantity > 0 && InventoryIndexMapByType.Contains(Item.ItemType))
		{
			int8 Empty = GetTypeInventoryEmptyIndex(Item.ItemType);
			if (Empty != -1)
				InventoryIndexMapByType[Item.ItemType][Empty] = ItemIdx;
		}
	}
}







