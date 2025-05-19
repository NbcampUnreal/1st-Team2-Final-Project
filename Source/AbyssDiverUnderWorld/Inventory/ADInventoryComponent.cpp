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
#include "Interactable/Item/Component/EquipUseComponent.h"

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
	DataTableSubsystem(nullptr)
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	InventorySizeByType = { 3, 2, 9, 1, 3 };

	ConstructorHelpers::FClassFinder<UAllInventoryWidget> AllInventoryWidget(TEXT("/Game/_AbyssDiver/Blueprints/UI/InventoryUI/WBP_AllInventoryWidget"));
	if (AllInventoryWidget.Succeeded())
	{
		InventoryWidgetClass = AllInventoryWidget.Class;
	}	

	for (int32 i = 0; i < static_cast<int32>(EItemType::Max); ++i)
	{
		InventoryIndexMapByType.FindOrAdd(static_cast<EItemType>(i));
		InventoryIndexMapByType[static_cast<EItemType>(i)].Init(-1, InventorySizeByType[i]);
	}

	InventoryList.SetOwningComponent(this);
}

void UADInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		DataTableSubsystem = GI->GetSubsystem<UDataTableSubsystem>();
	}
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
	GetWorld()->GetTimerManager().SetTimer(UseCoolTimeHandle, this, &UADInventoryComponent::OnUseCoolTimeEnd, 0.3f, false);


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
	
		FFADItemDataRow* FoundRow = DataTableSubsystem->GetItemData(Item.Id); 
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

void UADInventoryComponent::S_InventoryInitialize_Implementation()
{
	ServerSideInventoryInitialize();
}

void UADInventoryComponent::S_TransferSlots_Implementation(uint8 FromIndex, uint8 ToIndex)
{
	if (InventoryList.Items.IsValidIndex(FromIndex) && InventoryList.Items.IsValidIndex(ToIndex))
	{
		int8 TempSlotIndex = InventoryList.Items[ToIndex].SlotIndex;
		InventoryList.Items[ToIndex].SlotIndex = InventoryList.Items[FromIndex].SlotIndex;
		InventoryList.Items[FromIndex].SlotIndex = TempSlotIndex;
		InventoryList.MarkItemDirty(InventoryList.Items[FromIndex]);
		InventoryList.MarkItemDirty(InventoryList.Items[ToIndex]);
		InventoryUIUpdate();
	}
	else
	{
		LOG(TEXT("Invalid Inventory Index"));
	}
}

void UADInventoryComponent::S_RequestRemove_Implementation(uint8 InventoryIndex, int8 Count, bool bIsDropAction)
{
	RemoveInventoryItem(InventoryIndex, Count, bIsDropAction);
}

void UADInventoryComponent::S_RemoveBySlotIndex_Implementation(uint8 SlotIndex, EItemType ItemType, bool bIsDropAction)
{
	RemoveBySlotIndex(SlotIndex, ItemType, bIsDropAction);
}

void UADInventoryComponent::InventoryInitialize()
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		ServerSideInventoryInitialize(); // 인벤토리 초기화
	}

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
}

void UADInventoryComponent::AddInventoryItem(FItemData ItemData)
{
	if (TotalWeight + ItemData.Mass <= WeightMax)
	{
		FFADItemDataRow* FoundRow = DataTableSubsystem->GetItemDataByName(ItemData.Name); 
		if (FoundRow)
		{
			int8 ItemIndex = ItemData.ItemType == EItemType::Exchangable ? -1 : FindItemIndexByName(ItemData.Name);
			if (ItemIndex > -1) 
			{
		//		FItemData& Item = InventoryList.Items[ItemIndex];
		//		if (InventoryIndexMapByType.Contains(ItemData.ItemType) && GetTypeInventoryEmptyIndex(ItemData.ItemType) != -1 && Item.Quantity == 0)
		//		{
		//			Item.SlotIndex = GetTypeInventoryEmptyIndex(ItemData.ItemType);
		//		}
				if (FoundRow->Stackable)
				{
					bool bIsUpdateSuccess = InventoryList.UpdateQuantity(ItemIndex, ItemData.Quantity);
					if (bIsUpdateSuccess)
					{
						LOG(TEXT("Item Update, ItemName : %s, Id : %d"), *InventoryList.Items[ItemIndex].Name.ToString(), InventoryList.Items[ItemIndex].Id);
					}
					else
						LOG(TEXT("Update fail"));
		//				Item.Quantity += ItemData.Quantity;
		//			if (Item.ItemType == EItemType::Exchangable)
		//			{
		//				Item.Mass += ItemData.Mass;
		//				Item.Price += ItemData.Price;
		//				OnInventoryInfoUpdate(ItemData.Mass, ItemData.Price);
		//			}
				}
		//		else
		//		{
		//			if (Item.Quantity == 0)
		//			{
		//				++Item.Quantity;
		//			}
		//		}
		//		InventoryList.MarkItemDirty(Item); //FastArray 관련 함수
			}
			else
			{
				if (InventoryIndexMapByType.Contains(ItemData.ItemType) && GetTypeInventoryEmptyIndex(ItemData.ItemType) != -1)
				{
					uint8 SlotIndex = GetTypeInventoryEmptyIndex(ItemData.ItemType);
				
					FItemData NewItem = { FoundRow->Name, FoundRow->Id, ItemData.Quantity, SlotIndex, FoundRow->Amount, ItemData.Mass,ItemData.Price, FoundRow->ItemType, FoundRow->Thumbnail };
					if (InventoryIndexMapByType.Contains(NewItem.ItemType) && GetTypeInventoryEmptyIndex(NewItem.ItemType) != -1)
					{
						InventoryList.AddItem(NewItem);
						LOG(TEXT("Add New Item SlotIndex : %d"), SlotIndex);
					}
					if (ItemData.ItemType == EItemType::Exchangable)
					{
						OnInventoryInfoUpdate(NewItem.Mass, NewItem.Price);
					}
				}
				else
				{
					LOG(TEXT("%s Inventory is full"), *StaticEnum<EItemType>()->GetNameStringByValue((int64)ItemData.ItemType));
				}
			}
		}
	}
	InventoryUIUpdate();
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
			//InventoryUIUpdate();
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

void UADInventoryComponent::OnRep_InventoryList()
{
	InventoryUIUpdate();
}

int16 UADInventoryComponent::FindItemIndexByName(FName ItemID) //빈슬롯이 없으면 -1 반환
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

void UADInventoryComponent::RemoveInventoryItem(uint8 InventoryIndex, int8 Count, bool bIsDropAction)
{
	PrintLogInventoryData();
	if (InventoryList.Items.IsValidIndex(InventoryIndex))
	{
		FItemData& Item = InventoryList.Items[InventoryIndex];
		if (Count != -1 && Item.Quantity < Count) return;
		if (bIsDropAction)
		{
			int8 SpawnCount = Count != -1 ? Count : Item.Quantity;
			for(int i = 0; i< SpawnCount; ++i)
				DropItem(Item);
		}
		if (Count == -1)
		{
			InventoryList.UpdateQuantity(InventoryIndex, -Item.Quantity);
			if (Item.ItemType == EItemType::Exchangable)
			{
				OnInventoryInfoUpdate(-Item.Mass, -Item.Price);
			}
		}
		else
		{
			InventoryList.UpdateQuantity(InventoryIndex, -Count);
			//if (Item.ItemType != EItemType::Exchangable)
			//	Item.Quantity -= Count;
		}
		LOG(TEXT("Remove Inventory Index %d: Item : %s"), InventoryIndex, *Item.Name.ToString());
		if (Item.Quantity <= 0)
		{
			InventoryList.RemoveItem(InventoryIndex);
			//Item.SlotIndex = 99;
		}
	}
	else
	{
		LOG(TEXT("Invalid Inventory Index"));
	}

	InventoryUIUpdate();
}

void UADInventoryComponent::RemoveBySlotIndex(uint8 SlotIndex, EItemType ItemType, bool bIsDropAction)
{
	int8 InventoryIndex = -1;
	for (int i = 0; i < InventoryList.Items.Num(); ++i)
	{
		if (InventoryList.Items[i].ItemType == ItemType && InventoryList.Items[i].SlotIndex == SlotIndex)
			InventoryIndex = i;
	}
	if (InventoryList.Items.IsValidIndex(InventoryIndex))
	{
		FItemData& Item = InventoryList.Items[InventoryIndex];
		if (Item.Quantity < 1) return;

		if (bIsDropAction)
		{
			DropItem(Item);
		}
		InventoryList.UpdateQuantity(InventoryIndex, -1);

		LOGN(TEXT("Remove Inventory Index %d: Item : %s"), InventoryIndex, *Item.Name.ToString());
		if (Item.Quantity <= 0)
		{
			OnInventoryInfoUpdate(-Item.Mass, -Item.Price);
			InventoryList.RemoveItem(InventoryIndex);
		}
	}
	else
	{
		LOG(TEXT("Invalid Inventory Index"));
	}

	InventoryUIUpdate();
}


void UADInventoryComponent::ClientRequestInventoryInitialize()
{
	const bool bIsClient = (GetOwnerRole() < ROLE_Authority);
	if (bIsClient)
	{
		S_InventoryInitialize(); // 서버에 RPC 요청
	}
	InventoryInitialize(); // 클라에서 UI 생성
}

void UADInventoryComponent::InventoryUIUpdate()
{
	RebuildIndexMap();
	if (InventoryWidgetInstance)
	{
		LOG(TEXT("CLIENT"));
	}
	else
	{
		LOG(TEXT("NOT CLIENT"));
	}
	if (InventoryUpdateDelegate.IsBound())
	{
		InventoryInfoUpdateDelegate.Broadcast(TotalWeight, TotalPrice);
	}
	if (InventoryUpdateDelegate.IsBound())
	{
		InventoryUpdateDelegate.Broadcast();
	}
	PrintLogInventoryData();
}

int8 UADInventoryComponent::GetTypeInventoryEmptyIndex(EItemType ItemType)
{
	RebuildIndexMap();
	for (int i = 0; i < InventoryIndexMapByType[ItemType].Num(); ++i)
	{
		if (InventoryIndexMapByType[ItemType][i] == -1)
			return i;
	}
	return -1;
} 

FVector UADInventoryComponent::GetDropLocation()
{
	APlayerController* PC = Cast<APlayerController>(Cast<AADPlayerState>(GetOwner())->GetPlayerController());
	APawn* OwnerPawn = PC->GetPawn();
	FVector CameraForward = PC->PlayerCameraManager->GetCameraRotation().Vector();
	FVector DropLocation = OwnerPawn->GetActorLocation() +FVector(0, 0, 100) + UKismetMathLibrary::RandomUnitVectorInConeInDegrees(CameraForward, 30) * 350.0;
	return DropLocation;
}

FItemData UADInventoryComponent::CurrentEquipmentItemData()
{
	int8 Index = InventoryIndexMapByType[EItemType::Equipment][CurrentEquipmentIndex];
	return InventoryList.Items[Index];
}

void UADInventoryComponent::SetEquipInfo(int8 TypeInventoryIndex, AADUseItem* SpawnItem)
{
	CurrentEquipmentIndex = TypeInventoryIndex;
	CurrentEquipmentInstance = SpawnItem;
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
			SpawnedItem->SetItemInfo(ItemData, true);
			LOG(TEXT("EquipItem"));
			CurrentEquipmentInstance = SpawnedItem;
			SetEquipInfo(Index, SpawnedItem);

			if (UEquipUseComponent* EquipComp = Pawn->FindComponentByClass<UEquipUseComponent>()) // 나중에 Getter로 바꿔야 함
			{
				EquipComp->Initialize(SpawnedItem->ItemData.Id);
			}
		}
	}
}

void UADInventoryComponent::UnEquip()
{
	// EquipComp의 장비 현재값 초기화
	if (APlayerState* PS = Cast<APlayerState>(GetOwner()))
	{
		// GetPawn() returns the pawn possessed by this PlayerState
		if (APawn* Pawn = PS->GetPawn())
		{
			// Now find your EquipUseComponent on the pawn
			if (UEquipUseComponent* EquipComp = Pawn->FindComponentByClass<UEquipUseComponent>())
			{
				EquipComp->DeinitializeEquip();
			}
		}
	}

	if(CurrentEquipmentInstance)
		CurrentEquipmentInstance->Destroy();
	SetEquipInfo(-1, nullptr);
	LOG(TEXT("UnEquipItem"));
}

void UADInventoryComponent::DropItem(FItemData& ItemData)
{
	// EquipComp의 장비 현재값 초기화
	if (APlayerState* PS = Cast<APlayerState>(GetOwner()))
	{
		// GetPawn() returns the pawn possessed by this PlayerState
		if (APawn* Pawn = PS->GetPawn())
		{
			// Now find your EquipUseComponent on the pawn
			if (UEquipUseComponent* EquipComp = Pawn->FindComponentByClass<UEquipUseComponent>())
			{
				EquipComp->DeinitializeEquip();
			}
		}
	}
	AADUseItem* SpawnItem = GetWorld()->SpawnActor<AADUseItem>(AADUseItem::StaticClass(), GetDropLocation(), FRotator::ZeroRotator);
	SpawnItem->SetItemInfo(ItemData, false);
	LOGN(TEXT("SpawnItem"));
}

void UADInventoryComponent::OnInventoryInfoUpdate(int32 MassInfo, int32 PriceInfo)
{
	TotalWeight += MassInfo;
	TotalPrice += PriceInfo;
}

void UADInventoryComponent::RebuildIndexMap()
{
	// 모든 값을 -1로 초기화
	for (auto& Pair : InventoryIndexMapByType)
	{
		for (int8& Idx : Pair.Value)
			Idx = -1;
	}

	for (int16 ItemIdx = 0; ItemIdx < InventoryList.Items.Num(); ++ItemIdx)
	{
		const FItemData& Item = InventoryList.Items[ItemIdx];
		if (Item.SlotIndex != 99 && Item.Quantity > 0 && InventoryIndexMapByType.Contains(Item.ItemType))
		{
			InventoryIndexMapByType[Item.ItemType][Item.SlotIndex] = ItemIdx;
		}
	}
}

void UADInventoryComponent::OnUseCoolTimeEnd()
{
	bCanUseItem = true;
}

void UADInventoryComponent::ServerSideInventoryInitialize()
{
	//if (DataTableSubsystem)
	//{
	//	TArray<FFADItemDataRow*> ItemAllRows = DataTableSubsystem->GetItemDataTableArray();

	//	if (ItemAllRows.Num() > 0)
	//	{
	//		int8 ItemRowNum = ItemAllRows.Num();
	//		for (int8 i = 0; i < ItemRowNum; ++i)
	//		{
	//			FFADItemDataRow* FoundRow = ItemAllRows[i];
	//			if (FoundRow)
	//			{
	//				FItemData NewItem = { FoundRow->Name, FoundRow->Id, 0, 99, FoundRow->Amount, 0,0, FoundRow->ItemType, FoundRow->Thumbnail };
	//				if (InventoryIndexMapByType.Contains(FoundRow->ItemType) && GetTypeInventoryEmptyIndex(FoundRow->ItemType) != -1)
	//				{
	//					InventoryList.AddItem(NewItem);
	//					//LOG(TEXT("Id %d"), NewItem.Id);

	//				}
	//			}
	//		}
	//	}
		//PrintLogInventoryData();
	//}
}

void UADInventoryComponent::PrintLogInventoryData()
{
	for (int i = 0; i<InventoryList.Items.Num(); ++i)
	{
		LOGVN(Error, TEXT("Name : %s || Quntity : %d || SlotIndex : %d || InventoryIndex : %d"), *InventoryList.Items[i].Name.ToString(), InventoryList.Items[i].Quantity, InventoryList.Items[i].SlotIndex, i);
	}

	for (auto& Pair : InventoryIndexMapByType)
	{
		LOGVN(Warning,TEXT("ItemType : %d ############################################"), (int32)Pair.Key);
		for (int8 Index : Pair.Value)
		{
			if (Index >= 0)
			{
				LOGVN(Warning, TEXT("Item : %s, InventoryIndex : %d"), *InventoryList.Items[Index].Name.ToString(), Index);
			}
			else
				LOGVN(Warning, TEXT("NoItems"));
		}
	}
}








