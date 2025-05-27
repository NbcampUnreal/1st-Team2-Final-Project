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
#include "UI/ChargeBatteryWidget.h"
#include "Character/UnderwaterCharacter.h"

DEFINE_LOG_CATEGORY(InventoryLog);

UADInventoryComponent::UADInventoryComponent() :
	InventoryWidgetClass(nullptr),
	TotalWeight(0),
	TotalPrice(0),
	WeightMax(100),
	bInventoryWidgetShowed(false), 
	bAlreadyCursorShowed(false),
	bCanUseItem(true),
	CurrentEquipmentSlotIndex(INDEX_NONE),
	CurrentEquipmentInstance(nullptr),
	InventoryWidgetInstance(nullptr),
	DataTableSubsystem(nullptr),
	ChargeBatteryWidget(nullptr)
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

void UADInventoryComponent::S_UseInventoryItem_Implementation(EItemType ItemType, uint8 SlotIndex)
{
	if (ItemType == EItemType::Equipment)
	{
		if (SlotIndex > InventoryIndexMapByType[ItemType].Num()-1 || InventoryIndexMapByType[ItemType][SlotIndex] == -1 ) return;
	}

	if (!bCanUseItem) return;

	bCanUseItem = false;

	FTimerHandle UseCoolTimeHandle;
	float CoolTime = 0.3f; // 아이템 사용 쿨타임 설정
	GetWorld()->GetTimerManager().SetTimer(UseCoolTimeHandle, this, &UADInventoryComponent::OnUseCoolTimeEnd, CoolTime, false);

	int8 InventoryIndex = GetInventoryIndexByTypeAndSlotIndex(ItemType, SlotIndex);
	if (InventoryIndex == -1) return;
	FItemData& Item = InventoryList.Items[InventoryIndex];
	LOGINVEN(Warning, TEXT("UseItem %s"), *Item.Name.ToString());
	if (ItemType == EItemType::Equipment)
	{
		if (CurrentEquipmentSlotIndex == SlotIndex)
		{
			UnEquip();
		}
		else
		{
			if (CurrentEquipmentSlotIndex != INDEX_NONE)
			{
				UnEquip();
				LOGINVEN(Warning, TEXT("ChangeEquipment"));
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
				LOGINVEN(Warning, TEXT("Use Consumable Item %s"), *FoundRow->Name.ToString());
			}
		}
		RemoveBySlotIndex(SlotIndex, EItemType::Consumable, false);
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
	LOGINVEN(Warning, TEXT("Type: %s Transfer Slot %d -> %d"), *StaticEnum<EItemType>()->GetDisplayNameTextByValue((int64)SlotType).ToString(), FromIndex, ToIndex);
	InventoryUIUpdate();
}

void UADInventoryComponent::S_RemoveBySlotIndex_Implementation(uint8 SlotIndex, EItemType ItemType, bool bIsDropAction)
{
	RemoveBySlotIndex(SlotIndex, ItemType, bIsDropAction);
}

void UADInventoryComponent::S_EquipmentChargeBattery_Implementation(FName ItemName, int8 Amount)
{
	int8 Index = FindItemIndexByName(ItemName);

	InventoryList.UpdateAmount(Index, Amount);

	FFADItemDataRow* InItemMeta = DataTableSubsystem ? DataTableSubsystem->GetItemDataByName(ItemName) : nullptr;

	if (InventoryList.Items[Index].Amount >= InItemMeta->Amount)
	{
		InventoryList.SetAmount(Index, InItemMeta->Amount);
	}

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, ItemName]()
		{
			if (ChargeBatteryWidget)
				ChargeBatteryWidget->SetEquipBatteryAmount(ItemName, InventoryList.Items[FindItemIndexByName(ItemName)].Amount);
			C_SetEquipBatteryAmount(ItemName);
		}, 1.0f, false);

}

void UADInventoryComponent::S_UseBatteryAmount_Implementation(int8 Amount)
{
	int16 Index = FindItemIndexByName("Battery");
	if (Index != INDEX_NONE)
	{
		FFADItemDataRow* InItemMeta = DataTableSubsystem ? DataTableSubsystem->GetItemDataByName("Battery") : nullptr;
		InventoryList.UpdateAmount(Index, Amount);

		if (InventoryList.Items[Index].Amount == 0 && InItemMeta)
		{
			InventoryList.SetAmount(Index, InItemMeta->Amount); 
			RemoveBySlotIndex(InventoryList.Items[Index].SlotIndex, EItemType::Consumable, false);
		}
	}
}

void UADInventoryComponent::C_SetButtonActive_Implementation(FName CName, bool bCIsActive, int16 CAmount)
{
	if (ChargeBatteryWidget)
	{
		ChargeBatteryWidget->SetEquipBatteryButtonActivate(CName, bCIsActive);
		ChargeBatteryWidget->SetEquipBatteryAmount(CName, CAmount);
		if (bCIsActive)
		{
			LOGVN(Warning, TEXT("Activate %s Button"), *CName.ToString());
		}
		else
			LOGVN(Warning, TEXT("DeActivate %s Button"), *CName.ToString());
	}
}

void UADInventoryComponent::C_UpdateBatteryInfo_Implementation()
{
	ChargeBatteryWidget->UpdateBatteryInfo();
}

void UADInventoryComponent::C_SetEquipBatteryAmount_Implementation(FName CItemName)
{
	if (ChargeBatteryWidget)
		ChargeBatteryWidget->SetEquipBatteryAmount(CItemName, InventoryList.Items[FindItemIndexByName(CItemName)].Amount);
}

void UADInventoryComponent::InventoryInitialize()
{
	APlayerController* PC = Cast<APlayerController>(Cast<AADPlayerState>(GetOwner())->GetPlayerController());
	if (InventoryWidgetClass && PC && PC->IsLocalController())
	{
		InventoryWidgetInstance = CreateWidget<UToggleWidget>(PC, InventoryWidgetClass);
		LOGINVEN(Warning, TEXT("WidgetCreate!"));

		if (InventoryWidgetInstance)
		{
			InventoryWidgetInstance->AddToViewport();
			InventoryWidgetInstance->InitializeInventoriesInfo(this);
			InventoryWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

bool UADInventoryComponent::AddInventoryItem(FItemData ItemData)
{
	if (TotalWeight + ItemData.Mass <= WeightMax)
	{
		FFADItemDataRow* FoundRow = DataTableSubsystem->GetItemDataByName(ItemData.Name); 
		if (FoundRow)
		{
			int8 ItemIndex = ItemData.ItemType == EItemType::Exchangable ? -1 : FindItemIndexByName(ItemData.Name);
			bool bIsUpdateSuccess = false;
			if (ItemIndex > -1) 
			{
				if (FoundRow->Stackable)
				{
					bIsUpdateSuccess = InventoryList.UpdateQuantity(ItemIndex, ItemData.Quantity);
					InventoryList.SetAmount(ItemIndex, ItemData.Amount);
					if (bIsUpdateSuccess)
					{
						LOGINVEN(Warning, TEXT("Item Update, ItemName : %s, Id : %d"), *InventoryList.Items[ItemIndex].Name.ToString(), InventoryList.Items[ItemIndex].Id);
					}
					else
						LOGINVEN(Warning, TEXT("Update fail"));
				}
			}
			else
			{
				if (InventoryIndexMapByType.Contains(ItemData.ItemType) && GetTypeInventoryEmptyIndex(ItemData.ItemType) != INDEX_NONE)
				{
					bIsUpdateSuccess = true;
					uint8 SlotIndex = GetTypeInventoryEmptyIndex(ItemData.ItemType);
				
					FItemData NewItem = { FoundRow->Name, FoundRow->Id, ItemData.Quantity, SlotIndex, ItemData.Amount, ItemData.Mass,ItemData.Price, FoundRow->ItemType, FoundRow->Thumbnail };
					InventoryList.AddItem(NewItem);
					if (ItemData.ItemType == EItemType::Exchangable)
					{
						OnInventoryInfoUpdate(NewItem.Mass, NewItem.Price);
					}
					LOGINVEN(Warning, TEXT("Add New Item %s SlotIndex : %d"), *NewItem.Name.ToString(), SlotIndex);
				}
				else
				{
					LOGINVEN(Warning, TEXT("%s Inventory is full"), *StaticEnum<EItemType>()->GetNameStringByValue((int64)ItemData.ItemType));
				}
			}

			if (bIsUpdateSuccess)
			{
				InventoryUIUpdate();
				if (ItemData.Name == "NightVisionGoggle" || ItemData.Name == "DPV")
				{
					if (ChargeBatteryWidget)
					{
						ChargeBatteryWidget->SetEquipBatteryButtonActivate(ItemData.Name, true);
						ChargeBatteryWidget->SetEquipBatteryAmount(ItemData.Name, ItemData.Amount);
						LOGVN(Warning, TEXT("Activate %s Button"), *ItemData.Name.ToString());
					}
					C_SetButtonActive(ItemData.Name, true, ItemData.Amount);
				}
				else if (ItemData.Name == "Battery")
				{
					if (ChargeBatteryWidget)
					{
						ChargeBatteryWidget->UpdateBatteryInfo();
						LOGVN(Warning, TEXT("Acquire a battery"));
					}
					C_UpdateBatteryInfo();
				}
				return true;
			}
		}
	}
	return false;
	
}

void UADInventoryComponent::ShowInventory()
{
	APlayerController* PC = Cast<APlayerController>(Cast<AADPlayerState>(GetOwner())->GetPlayerController());
	if (!PC && !InventoryWidgetInstance) return;

	InventoryWidgetInstance->SetVisibility(ESlateVisibility::Visible);
	InventoryUIUpdate();
	bAlreadyCursorShowed = PC->bShowMouseCursor;
	PC->bShowMouseCursor = true;

	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(InventoryWidgetInstance->TakeWidget());

	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	PC->SetIgnoreLookInput(true);
	PC->SetInputMode(InputMode);
}

void UADInventoryComponent::HideInventory()
{
	APlayerController* PC = Cast<APlayerController>(Cast<AADPlayerState>(GetOwner())->GetPlayerController());
	if (!PC && !InventoryWidgetInstance) return;

	bInventoryWidgetShowed = false;
	InventoryWidgetInstance->SetVisibility(ESlateVisibility::Hidden);

	if (!bAlreadyCursorShowed)
		PC->bShowMouseCursor = false;
	PC->SetIgnoreLookInput(false);
	PC->SetInputMode(FInputModeGameOnly());
	
}

void UADInventoryComponent::OnRep_InventoryList()
{
	InventoryUIUpdate();
}

void UADInventoryComponent::OnRep_CurrentEquipmentSlotIndex()
{

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
	return INDEX_NONE;
}

void UADInventoryComponent::RemoveBySlotIndex(uint8 SlotIndex, EItemType ItemType, bool bIsDropAction)
{
	int8 InventoryIndex = GetInventoryIndexByTypeAndSlotIndex(ItemType, SlotIndex);
	if (InventoryIndex == INDEX_NONE) return;
	if (InventoryList.Items.IsValidIndex(InventoryIndex))
	{
		FItemData& Item = InventoryList.Items[InventoryIndex];

		if (Item.Quantity < 1) return;

		if (bIsDropAction)
		{
			DropItem(Item);
		}
		InventoryList.UpdateQuantity(InventoryIndex, INDEX_NONE);

		if (ItemType == EItemType::Equipment)
		{
			if (CurrentEquipmentSlotIndex != INDEX_NONE)
			{
				if (CurrentEquipmentSlotIndex == SlotIndex)
					UnEquip();
			}
		}
		if (Item.Name == "NightVisionGoggle" || Item.Name == "DPV")
		{
			if (ChargeBatteryWidget)
			{
				ChargeBatteryWidget->SetEquipBatteryButtonActivate(Item.Name, false);
				LOGINVEN(Warning, TEXT("DeActivate %s Button"), *Item.Name.ToString());
			}
			C_SetButtonActive(Item.Name, false, Item.Amount);
		}
		else if (Item.Name == "Battery")
		{
			InventoryList.SetAmount(InventoryIndex, 45);
			if (ChargeBatteryWidget)
			{
				ChargeBatteryWidget->UpdateBatteryInfo();
				LOGINVEN(Warning, TEXT("Remove a battery"));
			}
			C_UpdateBatteryInfo();
		}
		
		LOGINVEN(Warning, TEXT("Remove Inventory Slot Index %d: Item : %s"), InventoryIndex, *Item.Name.ToString());
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
		LOGINVEN(Warning, TEXT("Invalid Inventory Index"));
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
		if (InventoryIndexMapByType[ItemType][i] == INDEX_NONE)
			return i;
	}
	return INDEX_NONE;
} 

FVector UADInventoryComponent::GetDropLocation()
{
	APlayerController* PC = Cast<APlayerController>(Cast<AADPlayerState>(GetOwner())->GetPlayerController());
	APawn* OwnerPawn = PC->GetPawn();
	FVector CameraForward = PC->PlayerCameraManager->GetCameraRotation().Vector();
	FVector DropLocation = OwnerPawn->GetActorLocation() +FVector(0, 0, 100) + UKismetMathLibrary::RandomUnitVectorInConeInDegrees(CameraForward, 30) * 350.0;
	return DropLocation;
}

const FItemData* UADInventoryComponent::GetInventoryItemData(FName ItemNameToFind)
{
	int8 Index = FindItemIndexByName(ItemNameToFind);
	if (Index != INDEX_NONE)
		return &InventoryList.Items[Index];
	else
		return &EmptyItem;
}

FItemData* UADInventoryComponent::GetCurrentEquipmentItemData()
{
	int8 Index = GetInventoryIndexByTypeAndSlotIndex(EItemType::Equipment, CurrentEquipmentSlotIndex);
	if (Index == INDEX_NONE) return nullptr;
	return &InventoryList.Items[Index];
}

FItemData* UADInventoryComponent::GetEditableItemDataByName(FName ItemNameToEdit)
{
	int8 Index = FindItemIndexByName(ItemNameToEdit);
	if (Index != INDEX_NONE)
		return &InventoryList.Items[Index];
	return nullptr;
}

int8 UADInventoryComponent::GetInventoryIndexByTypeAndSlotIndex(EItemType Type, int8 SlotIndex) //못 찾으면 -1 반환
{
	int8 InventoryIndex = INDEX_NONE;
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
	if (CurrentEquipmentInstance)
	{
		LOGINVEN(Warning, TEXT("Slot Index : %d Item Name: %s"), SlotIndex, *SpawnItem->ItemData.Name.ToString());
	}
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

	ACharacter* Character = Cast<ACharacter>(Pawn);
	USkeletalMeshComponent* MeshComp = Character->GetMesh();

	if (MeshComp)
	{
		if (MeshComp->DoesSocketExist("Hand_R"))
		{
			LOGINVEN(Warning, TEXT("SpawnItem")); 
			AADUseItem* SpawnedItem = GetWorld()->SpawnActor<AADUseItem>(AADUseItem::StaticClass(), MeshComp->GetSocketLocation("Hand_R"), MeshComp->GetSocketRotation("Hand_R"), SpawnParams);
			if (SpawnedItem)
			{
				SpawnedItem->SetItemInfo(ItemData, true);
				SpawnedItem->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("Hand_R"));
				CurrentEquipmentInstance = SpawnedItem;
				LOGINVEN(Warning, TEXT("ItemToEquip Name: %s, Amount %d"), *ItemData.Name.ToString(), ItemData.Amount);
				SetEquipInfo(SlotIndex, SpawnedItem);

				if (UEquipUseComponent* EquipComp = Pawn->FindComponentByClass<UEquipUseComponent>()) // 나중에 Getter로 바꿔야 함
				{
					if(GetCurrentEquipmentItemData())
						EquipComp->Initialize(*GetCurrentEquipmentItemData());
				}
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
				InventoryList.MarkItemDirty(InventoryList.Items[FindItemIndexByName(CurrentEquipmentInstance->ItemData.Name)]);
			}
		}
	}

	LOGINVEN(Warning, TEXT("UnEquipItem %s"), *CurrentEquipmentInstance->ItemData.Name.ToString());
	if(CurrentEquipmentInstance)
		CurrentEquipmentInstance->Destroy();
	SetEquipInfo(INDEX_NONE, nullptr);
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
	LOGINVEN(Warning, TEXT("Spawn Item To Drop : %s"), *ItemData.Name.ToString());
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
			Idx = INDEX_NONE;
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

void UADInventoryComponent::EquipmentChargeBatteryUpdateDelay()
{
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

void UADInventoryComponent::SetChargeBatteryInstance(UChargeBatteryWidget* BatteryWidget)
{
	ChargeBatteryWidget = BatteryWidget;
	LOGINVEN(Warning, TEXT("Allocate BatteryWidget To Inventory"));
}





