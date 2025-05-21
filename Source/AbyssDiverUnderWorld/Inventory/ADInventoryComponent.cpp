// Fill out your copyright notice in the Description page of Project Settings.
#include "Inventory/ADInventoryComponent.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "DataRow/FADItemDataRow.h"
#include "UI/ToggleWidget.h"
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

DEFINE_LOG_CATEGORY(InventoryLog);

UADInventoryComponent::UADInventoryComponent() :
	InventoryWidgetClass(nullptr),
	TotalWeight(0),
	TotalPrice(0),
	WeightMax(100),
	bInventoryWidgetShowed(false), 
	bCanUseItem(true),
	CurrentEquipmentSlotIndex(-1),
	CurrentEquipmentInstance(nullptr),
	InventoryWidgetInstance(nullptr),
	DataTableSubsystem(nullptr)
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	InventorySizeByType = { 3, 2, 9, 1, 3 };

	ConstructorHelpers::FClassFinder<UToggleWidget> ToggleWidget(TEXT("/Game/_AbyssDiver/Blueprints/UI/InventoryUI/WBP_ToggleWidget"));
	if (ToggleWidget.Succeeded())
	{
		InventoryWidgetClass = ToggleWidget.Class;
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
	DOREPLIFETIME(UADInventoryComponent, CurrentEquipmentSlotIndex);
	DOREPLIFETIME(UADInventoryComponent, CurrentEquipmentInstance);
}

void UADInventoryComponent::S_UseInventoryItem_Implementation(EItemType ItemType, int32 SlotIndex)
{
	if (ItemType == EItemType::Equipment)
	{
		if (SlotIndex > InventoryIndexMapByType[ItemType].Num()-1 || InventoryIndexMapByType[ItemType][SlotIndex] == -1 ) return;
	}

	if (!bCanUseItem) return;

	bCanUseItem = false;

	FTimerHandle UseCoolTimeHandle;
	GetWorld()->GetTimerManager().SetTimer(UseCoolTimeHandle, this, &UADInventoryComponent::OnUseCoolTimeEnd, 0.3f, false);

	int8 InventoryIndex = GetInventoryIndexByTypeAndSlotIndex(ItemType, SlotIndex);
	if (InventoryIndex == -1) return;
	FItemData& Item = InventoryList.Items[InventoryIndex];
	LOGI(Warning, TEXT("UseItem %s"), *Item.Name.ToString());
	if (ItemType == EItemType::Equipment)
	{
		if (CurrentEquipmentSlotIndex == SlotIndex)
		{
			UnEquip();
		}
		else
		{
			if (CurrentEquipmentSlotIndex != -1)
			{
				UnEquip();
				LOGI(Warning, TEXT("ChangeEquipment"));
			}
			Equip(Item, SlotIndex);
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
				LOGI(Warning, TEXT("Use Consumable Item"));
			}
		}
		RemoveInventoryItem(SlotIndex, 1, false);
	}
}

void UADInventoryComponent::S_TransferSlots_Implementation(EItemType SlotType, uint8 FromIndex, uint8 ToIndex)
{
	int8 FromInventoryIndex = GetInventoryIndexByTypeAndSlotIndex(SlotType, FromIndex);
	int8 ToInventoryIndex = GetInventoryIndexByTypeAndSlotIndex(SlotType, ToIndex);

	if (!InventoryList.Items.IsValidIndex(ToInventoryIndex))
	{
		InventoryList.Items[FromInventoryIndex].SlotIndex = ToIndex;
		InventoryList.MarkItemDirty(InventoryList.Items[FromInventoryIndex]);
	}
	else
	{
		InventoryList.Items[FromInventoryIndex].SlotIndex = ToIndex;
		InventoryList.Items[ToInventoryIndex].SlotIndex = FromIndex;
		InventoryList.MarkItemDirty(InventoryList.Items[FromInventoryIndex]);
		InventoryList.MarkItemDirty(InventoryList.Items[ToInventoryIndex]);
	}
	LOGI(Warning, TEXT("Transfer Slot"));
	InventoryUIUpdate();
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
	APlayerController* PC = Cast<APlayerController>(Cast<AADPlayerState>(GetOwner())->GetPlayerController());
	if (InventoryWidgetClass && PC && PC->IsLocalController())
	{
		InventoryWidgetInstance = CreateWidget<UToggleWidget>(PC, InventoryWidgetClass);
		LOGI(Warning, TEXT("WidgetCreate!"));

		if (InventoryWidgetInstance)
		{
			InventoryWidgetInstance->AddToViewport();
			InventoryWidgetInstance->InitializeInventoriesInfo(this);
			InventoryWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
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
				if (FoundRow->Stackable)
				{
					bool bIsUpdateSuccess = InventoryList.UpdateQuantity(ItemIndex, ItemData.Quantity);
					if (bIsUpdateSuccess)
					{
						LOGI(Warning, TEXT("Item Update, ItemName : %s, Id : %d"), *InventoryList.Items[ItemIndex].Name.ToString(), InventoryList.Items[ItemIndex].Id);
					}
					else
						LOGI(Warning, TEXT("Update fail"));
				}
			}
			else
			{
				if (InventoryIndexMapByType.Contains(ItemData.ItemType) && GetTypeInventoryEmptyIndex(ItemData.ItemType) != -1)
				{
					uint8 SlotIndex = GetTypeInventoryEmptyIndex(ItemData.ItemType);
				
					FItemData NewItem = { FoundRow->Name, FoundRow->Id, ItemData.Quantity, SlotIndex, ItemData.Amount, ItemData.Mass,ItemData.Price, FoundRow->ItemType, FoundRow->Thumbnail };
					if (InventoryIndexMapByType.Contains(NewItem.ItemType) && GetTypeInventoryEmptyIndex(NewItem.ItemType) != -1)
					{
						InventoryList.AddItem(NewItem);
						LOGI(Warning, TEXT("Add New Item %s SlotIndex : %d"), *NewItem.Name.ToString(), SlotIndex);
					}
					if (ItemData.ItemType == EItemType::Exchangable)
					{
						OnInventoryInfoUpdate(NewItem.Mass, NewItem.Price);
					}
				}
				else
				{
					LOGI(Warning, TEXT("%s Inventory is full"), *StaticEnum<EItemType>()->GetNameStringByValue((int64)ItemData.ItemType));
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
			InventoryWidgetInstance->SetVisibility(ESlateVisibility::Hidden);

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
	//PrintLogInventoryData();
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
	int8 InventoryIndex = GetInventoryIndexByTypeAndSlotIndex(ItemType, SlotIndex);
	if (InventoryIndex == -1) return;
	if (ItemType == EItemType::Equipment && CurrentEquipmentSlotIndex != -1)
	{
		if (CurrentEquipmentSlotIndex == SlotIndex)
			UnEquip();
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
			if (Item.ItemType == EItemType::Exchangable)
			{
				OnInventoryInfoUpdate(-Item.Mass, -Item.Price);
			}
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
	InventoryInitialize(); // 클라에서 UI 생성
}

void UADInventoryComponent::InventoryUIUpdate()
{
	RebuildIndexMap();
	if (InventoryUpdateDelegate.IsBound())
	{
		InventoryInfoUpdateDelegate.Broadcast(TotalWeight, TotalPrice);
	}
	if (InventoryUpdateDelegate.IsBound())
	{
		InventoryUpdateDelegate.Broadcast();
	}
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

FItemData* UADInventoryComponent::GetCurrentEquipmentItemData()
{
	int8 Index = GetInventoryIndexByTypeAndSlotIndex(EItemType::Equipment, CurrentEquipmentSlotIndex);
	if (Index == -1) return nullptr;
	return &InventoryList.Items[Index];
}

int8 UADInventoryComponent::GetInventoryIndexByTypeAndSlotIndex(EItemType Type, int8 SlotIndex) //못 찾으면 -1 반환
{
	int8 InventoryIndex = -1;
	for (int i = 0; i < InventoryList.Items.Num(); ++i)
	{
		if (InventoryList.Items[i].ItemType == Type && InventoryList.Items[i].SlotIndex == SlotIndex)
			InventoryIndex = i;
	}
	return InventoryIndex;
}


void UADInventoryComponent::SetEquipInfo(int8 SlotIndex, AADUseItem* SpawnItem)
{
	CurrentEquipmentSlotIndex = SlotIndex;
	CurrentEquipmentInstance = SpawnItem;
	LOGI(Warning, TEXT("Slot Index : %d Item Name: %s"), SlotIndex, *SpawnItem->ItemData.Name.ToString());
}

void UADInventoryComponent::Equip(FItemData& ItemData, int8 SlotIndex)
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
	LOGI(Warning, TEXT("SpawnItem")); 
	if (SpawnedItem)
	{
		ACharacter* Character = Cast<ACharacter>(Pawn);
		USkeletalMeshComponent* MeshComp = Character->GetMesh();
		if (MeshComp)
		{
			SpawnedItem->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("Hand_R"));
			SpawnedItem->SetItemInfo(ItemData, true);
			CurrentEquipmentInstance = SpawnedItem;
			LOGI(Warning, TEXT("ItemToEquip Amount %d"), ItemData.Amount);
			SetEquipInfo(SlotIndex, SpawnedItem);

			if (UEquipUseComponent* EquipComp = Pawn->FindComponentByClass<UEquipUseComponent>()) // 나중에 Getter로 바꿔야 함
			{
				if(GetCurrentEquipmentItemData())
					EquipComp->Initialize(*GetCurrentEquipmentItemData());
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
	LOGI(Warning, TEXT("UnEquipItem"));
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
	LOGI(Warning, TEXT("Spawn Item To Drop : %s"), *ItemData.Name.ToString());
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








