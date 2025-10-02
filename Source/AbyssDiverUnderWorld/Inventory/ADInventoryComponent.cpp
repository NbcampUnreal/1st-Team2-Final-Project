// Fill out your copyright notice in the Description page of Project Settings.
#include "Inventory/ADInventoryComponent.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "DataRow/FADItemDataRow.h"
#include "UI/ToggleWidget.h"
#include "Net/UnrealNetwork.h"
#include "AbyssDiverUnderWorld.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Subsystems/DataTableSubsystem.h"
#include "Framework/ADPlayerState.h"
#include "Interactable/Item/ADUseItem.h"
#include "Interactable/Item/UseFunction/UseStrategy.h"
//#include "Actions/PawnActionsComponent.h"
#include "GameFramework/Character.h"
#include "Interactable/Item/Component/EquipUseComponent.h"
#include "UI/ChargeBatteryWidget.h"
#include "Character/UnderwaterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/ADInGameState.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Subsystems/SoundSubsystem.h"
#include "Framework/ADGameInstance.h"
#include "Framework/ADTutorialGameMode.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interactable/EquipableComponent/EquipRenderComponent.h"
#include "Character/PlayerComponent/PlayerHUDComponent.h"
#include "Missions/MissionBase.h"

DEFINE_LOG_CATEGORY(InventoryLog);

static FORCEINLINE FGameplayTag RequestTag(const TCHAR* Root, const FString& Tail)
{
	return UGameplayTagsManager::Get().RequestGameplayTag(
		FName(*FString::Printf(TEXT("%s.%s"), Root, *Tail)), /*ErrorIfNotFound*/ false);
}

UADInventoryComponent::UADInventoryComponent() :
	ToggleWidgetClass(nullptr),
	TotalWeight(0),
	TotalPrice(0),
	CurrentEquipmentSlotIndex(INDEX_NONE),
	CurrentEquipmentInstance(nullptr),
	ToggleWidgetInstance(nullptr),
	WeightMax(100),
	bCanUseItem(true),
	DataTableSubsystem(nullptr),
	ChargeBatteryWidget(nullptr)
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	InventorySizeByType = { 3, 2, 9, 1, 3 };

	ConstructorHelpers::FClassFinder<UToggleWidget> ToggleWidget(TEXT("/Game/_AbyssDiver/Blueprints/UI/InventoryUI/WBP_ToggleWidget"));
	if (ToggleWidget.Succeeded())
	{
		ToggleWidgetClass = ToggleWidget.Class;
	}	

	for (int32 i = 0; i < static_cast<int8>(EItemType::Max); ++i)
	{
		InventoryIndexMapByType.FindOrAdd(static_cast<EItemType>(i));
		InventoryIndexMapByType[static_cast<EItemType>(i)].Init(-1, InventorySizeByType[i]);
	}

	ConstructorHelpers::FObjectFinder<UNiagaraSystem> OxygenRefillEffectFinder(
		TEXT("/Game/_AbyssDiver/FX/VFX/Item/NS_RefillOxygen.NS_RefillOxygen")
	);
	if (OxygenRefillEffectFinder.Succeeded())
	{
		OxygenRefillEffect = OxygenRefillEffectFinder.Object;
	}
	
	ConstructorHelpers::FObjectFinder<UNiagaraSystem> DropItemEffectFinder(
		TEXT("/Game/_AbyssDiver/FX/VFX/Item/NS_DropItemBubble.NS_DropItemBubble")
	);
	if (DropItemEffectFinder.Succeeded())
	{
		DropItemEffect = DropItemEffectFinder.Object;
	}
}

void UADInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		DataTableSubsystem = GI->GetSubsystem<UDataTableSubsystem>();
		SoundSubsystem = GI->GetSubsystem<USoundSubsystem>();
	}
	
	TryCachedDiver();
	SetComponentTickEnabled(true);

}

void UADInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// CachedDiver가 아직 null일 때만 재시도
	if (!CachedDiver)
	{
		TryCachedDiver();
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
	DOREPLIFETIME(UADInventoryComponent, CurrentEquipItem);
	DOREPLIFETIME(UADInventoryComponent, bIsWeapon);
	DOREPLIFETIME(UADInventoryComponent, EquipmentType);
	DOREPLIFETIME(UADInventoryComponent, CachedDiver);
}

void UADInventoryComponent::S_UseInventoryItem_Implementation(EItemType ItemType, uint8 SlotIndex, bool bIgnoreCoolTime)
{
	if (APlayerController* PC = Cast<APlayerController>(Cast<AADPlayerState>(GetOwner())->GetPlayerController()))
	{
		if (AUnderwaterCharacter* Character = Cast<AUnderwaterCharacter>(PC->GetPawn()))
		{
			if (Character->GetEnvironmentState() == EEnvironmentState::Ground)
			{
				return;
			}
				
		}
	}
	if (ItemType == EItemType::Equipment)
	{
		if (SlotIndex > InventoryIndexMapByType[ItemType].Num()-1 || InventoryIndexMapByType[ItemType][SlotIndex] == -1 ) return;
	}

	if (!bCanUseItem) return;

	if (!bIgnoreCoolTime)
	{
		bCanUseItem = false;

		FTimerHandle UseCoolTimeHandle;
		float CoolTime = 0.3f; // 아이템 사용 쿨타임 설정
		GetWorld()->GetTimerManager().SetTimer(UseCoolTimeHandle, this, &UADInventoryComponent::OnUseCoolTimeEnd, CoolTime, false);
	}
	
	int8 InventoryIndex = GetInventoryIndexByTypeAndSlotIndex(ItemType, SlotIndex);
	if (InventoryIndex == -1) return;
	FItemData& Item = InventoryList.Items[InventoryIndex];
	LOGINVEN(Warning, TEXT("TryUseItem %s"), *Item.Name.ToString());
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
				if (Strategy->Use(GetOwner()))
				{
					RemoveBySlotIndex(SlotIndex, EItemType::Consumable, false);
					if (Item.Id == 1)
					{
						C_InventoryPlaySound(ESFX::Breath);
						FTimerHandle SpawnEffectDelay;
						GetWorld()->GetTimerManager().SetTimer(SpawnEffectDelay, this, &UADInventoryComponent::C_SpawnItemEffect, 1.5f, false);
					}
					if (UMissionEventHubComponent* Hub = GetMissionHub())
					{
						FGameplayTagContainer Tags = MakeItemTags(Item.Name);
						Hub->BroadcastItemUsed(Tags, 1);

					}

					LOGINVEN(Warning, TEXT("Use Consumable Item %s"), *FoundRow->Name.ToString());
				}
				else
				{
					if (Item.Id == 0)
					{
						C_OnShieldUseFailed();
					}
					LOGINVEN(Warning, TEXT("Use Consumable Item Failed %s"), *FoundRow->Name.ToString());
				}
			}
		}
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

void UADInventoryComponent::S_EquipmentChargeBattery_Implementation(EChargeBatteryType ItemChargeBatteryType, int8 Amount)
{
	int8 Index = FindItemIndexByID(static_cast<int8>(ItemChargeBatteryType));

	InventoryList.UpdateAmount(Index, Amount);

	FFADItemDataRow* InItemMeta = DataTableSubsystem ? DataTableSubsystem->GetItemData(static_cast<int8>(ItemChargeBatteryType)) : nullptr;

	if (InventoryList.Items[Index].Amount >= InItemMeta->Amount)
	{
		InventoryList.SetAmount(Index, InItemMeta->Amount);
	}

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, ItemChargeBatteryType]()
		{
			if (ChargeBatteryWidget)
				ChargeBatteryWidget->SetEquipBatteryAmount(ItemChargeBatteryType, InventoryList.Items[FindItemIndexByID(static_cast<int8>(ItemChargeBatteryType))].Amount);
			C_SetEquipBatteryAmount(ItemChargeBatteryType);
		}, 1.0f, false);

}

void UADInventoryComponent::S_UseBatteryAmount_Implementation(int8 Amount)
{
	int16 Index = FindItemIndexByName("Battery");
	if (Index != INDEX_NONE)
	{
		FFADItemDataRow* InItemMeta = DataTableSubsystem ? DataTableSubsystem->GetItemDataByName("Battery") : nullptr;
		InventoryList.UpdateAmount(Index, Amount);

		if (InventoryList.Items[Index].Amount <= 0 && InItemMeta)
		{
			InventoryList.SetAmount(Index, InItemMeta->Amount); 
			RemoveBySlotIndex(InventoryList.Items[Index].SlotIndex, EItemType::Consumable, false);
		}
	}
}

void UADInventoryComponent::M_SpawnItemEffect_Implementation(ESFX SFXType, UNiagaraSystem* VFX, FVector SpawnLocation)
{
	if (!VFX) return;
	GetSoundSubsystem()->PlayAt(SFXType, SpawnLocation);
	
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		VFX,
		SpawnLocation,
		FRotator::ZeroRotator,
		FVector(1.0f),
		true,  // bAutoDestroy
		true,  // bAutoActivate
		ENCPoolMethod::None,
		true   // bPreCullCheck
	);
}

void UADInventoryComponent::C_InventoryPlaySound_Implementation(ESFX SFXType)
{
	GetSoundSubsystem()->Play2D(SFXType);
}

void UADInventoryComponent::C_SpawnItemEffect_Implementation()
{
	//UObject는 리플리케이트를 지원하지 않으므로, 이펙트 스폰을 클라이언트에서만 실행되는 함수로 구현

	GetSoundSubsystem()->Play2D(ESFX::RefillOxygen);
	if (!OxygenRefillEffect) return;
	if (APlayerController* PC = Cast<APlayerController>(Cast<AADPlayerState>(GetOwner())->GetPlayerController()))
	{
		FVector Velocity;
		if (APawn* OwnerPawn = PC->GetPawn())
		{
			UCharacterMovementComponent* MoveComp = Cast<UCharacterMovementComponent>(OwnerPawn->GetMovementComponent());
			Velocity = MoveComp->Velocity;
		}
		FVector CamLocation;
		FRotator CamRotation;

		PC->GetPlayerViewPoint(CamLocation, CamRotation);
		FVector SpawnLocation = CamLocation + CamRotation.Vector() * 5.f - FVector(0, 0, 10);
		if (!Velocity.IsNearlyZero())
		{
			SpawnLocation = SpawnLocation + CamRotation.Vector() * 30.f;
		}
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			OxygenRefillEffect,
			SpawnLocation,
			CamRotation,
			FVector(1.0f),
			true,  // bAutoDestroy
			true,  // bAutoActivate
			ENCPoolMethod::None,
			true   // bPreCullCheck
		);
	}
}

void UADInventoryComponent::C_SetButtonActive_Implementation(EChargeBatteryType ItemChargeBatteryType, bool bCIsActive, int16 CAmount)
{
	if (ChargeBatteryWidget)
	{
		ChargeBatteryWidget->SetEquipBatteryButtonActivate(ItemChargeBatteryType, bCIsActive);
		ChargeBatteryWidget->SetEquipBatteryAmount(ItemChargeBatteryType, CAmount);
		if (bCIsActive)
		{
			LOGINVEN(Warning, TEXT("Activate Button"));
		}
		else
			LOGINVEN(Warning, TEXT("DeActivate Button"));
	}
}

void UADInventoryComponent::C_UpdateBatteryInfo_Implementation()
{
	if(ChargeBatteryWidget)
		ChargeBatteryWidget->UpdateBatteryInfo();
}

void UADInventoryComponent::C_OnShieldUseFailed_Implementation()
{
	APlayerState* PS = Cast<APlayerState>(GetOwner());
	if (!PS) return;
	AADPlayerController* PC = Cast<AADPlayerController>(PS->GetOwningController());
	if (!PC) return;

	UPlayerHUDComponent* HUD = PC->GetPlayerHUDComponent();

	HUD->OnShieldUseFailed();
}

void UADInventoryComponent::C_SetEquipBatteryAmount_Implementation(EChargeBatteryType ItemChargeBatteryType)
{
	if (ChargeBatteryWidget)
		ChargeBatteryWidget->SetEquipBatteryAmount(ItemChargeBatteryType, InventoryList.Items[FindItemIndexByID(static_cast<int8>(ItemChargeBatteryType))].Amount);
}

void UADInventoryComponent::InventoryInitialize()
{
	APlayerController* PC = Cast<APlayerController>(Cast<AADPlayerState>(GetOwner())->GetPlayerController());
	if (!ToggleWidgetClass || !PC || !PC->IsLocalController())
	{
		return;
	}

	ToggleWidgetInstance = CreateWidget<UToggleWidget>(PC, ToggleWidgetClass);
	LOGINVEN(Warning, TEXT("WidgetCreate!"));

	if (!ToggleWidgetInstance)
	{
		LOGINVEN(Warning, TEXT("!ToggleWidgetInstance"));
		return;
	}

	ToggleWidgetInstance->AddToViewport(1);
	ToggleWidgetInstance->InitializeInventoriesInfo(this);
	ToggleWidgetInstance->SetVisibility(ESlateVisibility::Hidden);

	InventoryUIUpdate();
}

bool UADInventoryComponent::AddInventoryItem(const FItemData& ItemData)
{
	if (TotalWeight + ItemData.Mass <= WeightMax)
	{
		if (DataTableSubsystem)
		{
			FFADItemDataRow* FoundRow = DataTableSubsystem->GetItemData(ItemData.Id);
			if (FoundRow)
			{
				int8 ItemIndex = FindItemIndexByName(ItemData.Name);
				if (ItemData.ItemType == EItemType::Equipment)
				{
					if (ItemIndex != -1)
					{
						return false;
					}		
				}
				LOGINVEN(Warning, TEXT("AddInventoryItem ItemIndex : %d"), ItemIndex);
				bool bIsUpdateSuccess = false;

				if (FoundRow->Stackable && ItemIndex != -1)
				{
					bIsUpdateSuccess = InventoryList.UpdateQuantity(ItemIndex, ItemData.Quantity);
					if (InventoryList.Items[ItemIndex].Amount > ItemData.Amount)
						InventoryList.SetAmount(ItemIndex, ItemData.Amount);
					if (bIsUpdateSuccess)
					{
						LOGINVEN(Warning, TEXT("Item Update, ItemName : %s, Id : %d"), *InventoryList.Items[ItemIndex].Name.ToString(), InventoryList.Items[ItemIndex].Id);
					}
					else
						LOGINVEN(Warning, TEXT("Update fail"));
				}
				else
				{
					if (InventoryIndexMapByType.Contains(ItemData.ItemType) && GetTypeInventoryEmptyIndex(ItemData.ItemType) != INDEX_NONE)
					{
						bIsUpdateSuccess = true;
						uint8 SlotIndex = GetTypeInventoryEmptyIndex(ItemData.ItemType);

						FItemData NewItem = { FoundRow->Name, FoundRow->Id, ItemData.Quantity, SlotIndex, ItemData.Amount, ItemData.CurrentAmmoInMag, ItemData.ReserveAmmo, ItemData.Mass,ItemData.Price, FoundRow->ItemType, FoundRow->BulletType, FoundRow->Thumbnail };
						InventoryList.AddItem(NewItem);
						//자원 아이템일 경우
						if (ItemData.ItemType == EItemType::Exchangable)
						{
							OnInventoryInfoUpdate(NewItem.Mass, NewItem.Price);
							//Mission Event Hub에 아이템 획득 이벤트 브로드캐스트
							if (UMissionEventHubComponent* Hub = GetMissionHub())
							{
								FGameplayTagContainer Tags = MakeItemTags(NewItem.Name);
								Hub->BroadcastItemCollected(Tags, NewItem.Mass);
							}
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
					CheckItemsForBattery();
					return true;
				}
			}
		}
	}
	return false;
	
}

void UADInventoryComponent::ShowInventory()
{
	AADPlayerState* PS = Cast<AADPlayerState>(GetOwner());
	if (!PS) return;
	APlayerController* PC = Cast<APlayerController>(PS->GetPlayerController());
	if (!PC || !ToggleWidgetInstance) return;

	ToggleWidgetInstance->PlaySlideAnimation(true);
	PC->bShowMouseCursor = true;

	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(ToggleWidgetInstance->TakeWidget());

	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	PC->SetIgnoreLookInput(true);
	PC->SetInputMode(InputMode);
}

void UADInventoryComponent::HideInventory()
{
	APlayerController* PC = Cast<APlayerController>(Cast<AADPlayerState>(GetOwner())->GetPlayerController());
	if (!PC || !ToggleWidgetInstance) return;

	ToggleWidgetInstance->PlaySlideAnimation(false);

	PC->bShowMouseCursor = false;
	PC->SetIgnoreLookInput(false);
	PC->SetInputMode(FInputModeGameOnly());
	
}

void UADInventoryComponent::OnRep_InventoryList()
{
	InventoryUIUpdate();
}

void UADInventoryComponent::OnRep_CurrentEquipItem()
{
	if (!CachedDiver)
	{
		LOGINVEN(Warning, TEXT("CachedDiver is null"));
		return;
	}

	UEquipRenderComponent* EquipRenderComp = CachedDiver->GetEquipRenderComponent();
	if (!EquipRenderComp)
	{
		LOGINVEN(Warning, TEXT("EquipRenderComp is null"));
		return;
	}

	if (PrevEquipItem)
	{
		EquipRenderComp->DetachItem(PrevEquipItem);
		PrevEquipItem->Destroy();
		PrevEquipItem = nullptr;
	}
	if (CurrentEquipItem)
	{
		FName Socket = NAME_None;
		switch (EquipmentType)
		{
		case EEquipmentType::HarpoonGun:	Socket = HarpoonSocketName; break;
		case EEquipmentType::FlareGun:		Socket = FlareSocketName;   break;
		case EEquipmentType::DPV:			Socket = DPVSocketName;     break;
		case EEquipmentType::Shotgun:		Socket = ShotgunSocketName; break;
		case EEquipmentType::Mine:			Socket = MineSocketName;    break;
		case EEquipmentType::ToyHammer:     Socket = HammerSocketName;  break;
		default:														break;
		}

		if (USkeletalMeshComponent* SkeletalMeshComp = CurrentEquipItem->FindComponentByClass<USkeletalMeshComponent>())
		{
			SkeletalMeshComp->SetVisibility(false, true);
			EquipRenderComp->AttachItem(CurrentEquipItem, Socket);
		}
		PrevEquipItem = CurrentEquipItem;
	}
}

int8 UADInventoryComponent::FindItemIndexByName(FName ItemName) //빈슬롯이 없으면 -1 반환
{
	for (int i = 0; i < InventoryList.Items.Num(); ++i)
	{
		if (InventoryList.Items[i].Name == ItemName)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

int8 UADInventoryComponent::FindItemIndexByID(int8 ItemID)
{
	for (int i = 0; i < InventoryList.Items.Num(); ++i)
	{
		if (InventoryList.Items[i].Id == ItemID)
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
			if (AADTutorialGameMode* GM = GetWorld()->GetAuthGameMode<AADTutorialGameMode>())
			{
				GM->OnPlayerItemAction(EPlayerActionTrigger::Drop);
			}
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
				ChargeBatteryWidget->SetEquipBatteryButtonActivate(static_cast<EChargeBatteryType>(Item.Id), false);
				LOGINVEN(Warning, TEXT("DeActivate %s Button"), *Item.Name.ToString());
			}
			C_SetButtonActive(static_cast<EChargeBatteryType>(Item.Id), false, Item.Amount);
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
	LOGINVEN(Warning, TEXT("InventoryUIUpdate"));
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

void UADInventoryComponent::CopyInventoryFrom(UADInventoryComponent* Source)
{
	if (!Source) return;
	LOGINVEN(Warning, TEXT("CopyInventory"));
	InventoryList.Items.Empty();

	for (const FItemData& Item : Source->InventoryList.Items)
	{
		InventoryList.Items.Add(Item);
	}
	Source->TotalPrice = TotalPrice;
	Source->TotalWeight = TotalWeight;
	// FastArray는 복사 후 MarkItemDirty 필요
	InventoryMarkArrayDirty();

	FTimerHandle UpdateDelayTimerHandle;
	float UpdateDelay = 0.2f;
	GetWorld()->GetTimerManager().SetTimer(UpdateDelayTimerHandle, this, &UADInventoryComponent::InventoryUIUpdate, UpdateDelay, false);

}

void UADInventoryComponent::InventoryMarkArrayDirty()
{
	for (int32 i = 0; i < InventoryList.Items.Num(); ++i)
	{
		InventoryList.MarkItemDirty(InventoryList.Items[i]);
	}
}

void UADInventoryComponent::CheckItemsForBattery()
{
	for (const FItemData& Item : InventoryList.Items)
	{
		if (Item.Name == "NightVisionGoggle" || Item.Name == "DPV")
		{
			if (ChargeBatteryWidget)
			{
				ChargeBatteryWidget->SetEquipBatteryButtonActivate(static_cast<EChargeBatteryType>(Item.Id), true);
				ChargeBatteryWidget->SetEquipBatteryAmount(static_cast<EChargeBatteryType>(Item.Id), Item.Amount);
				LOGVN(Warning, TEXT("Activate %s Button"), *Item.Name.ToString());
			}
			C_SetButtonActive(static_cast<EChargeBatteryType>(Item.Id), true, Item.Amount);
		}
		else if (Item.Name == "Battery")
		{
			if (ChargeBatteryWidget)
			{
				ChargeBatteryWidget->UpdateBatteryInfo();
				LOGVN(Warning, TEXT("Acquire a battery"));
			}
			C_UpdateBatteryInfo();
		}
	}
}

void UADInventoryComponent::PlayEquipAnimation(AUnderwaterCharacter* Character, EEquipmentType InType)
{
	if (!Character) return;

	UAnimMontage* Montage = GetDrawMontageByType(InType);
	if (!Montage) return; // 해당 타입 몽타주가 없으면 스킵

	FAnimSyncState SyncState;
	SyncState.bEnableRightHandIK = true;
	SyncState.bEnableLeftHandIK = false;
	SyncState.bEnableFootIK = true;
	SyncState.bIsStrafing = false;

	Character->M_StopAllMontagesOnBothMesh(0.f);
	Character->M_PlayMontageOnBothMesh(Montage, 1.0f, NAME_None, SyncState);
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
	FVector DropLocation = OwnerPawn->GetActorLocation() +FVector(0, 0, 50) + UKismetMathLibrary::RandomUnitVectorInConeInDegrees(CameraForward, 30) * 150.0;
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

bool UADInventoryComponent::TryGiveAmmoToEquipment(EBulletType BulletType, int32 AmountPerPickup)
{
	for (FItemData& Item : InventoryList.Items)  
	{
		if (Item.ItemType == EItemType::Equipment &&
			Item.BulletType == BulletType)
		{
			Item.ReserveAmmo += AmountPerPickup;
			Item.Amount += AmountPerPickup;
			InventoryList.MarkItemDirty(Item); 
			return true;      
		}
	}
	return false;
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
	CachedDiver = Cast<AUnderwaterCharacter>(Pawn);
	UEquipRenderComponent* EquipRenderComp = CachedDiver->GetEquipRenderComponent();
	if (!Character || !CachedDiver || !EquipRenderComp)
		return;

	if (MeshComp)
	{
		if (MeshComp->DoesSocketExist(HarpoonSocketName) && MeshComp->DoesSocketExist(DPVSocketName))
		{
			//const FName SocketName = bIsWeapon ? HarpoonSocketName : DPVSocketName;
			//FTransform AttachTM = MeshComp->GetSocketTransform(SocketName);

			AADUseItem* SpawnedItem = GetWorld()->SpawnActor<AADUseItem>(
				AADUseItem::StaticClass(),
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				SpawnParams
			);
			if (!SpawnedItem)
			{
				LOGINVEN(Warning, TEXT("No SpawnItem"));
				return;
			}

			C_InventoryPlaySound(ESFX::Equip);
			SpawnedItem->SetItemInfo(ItemData, true, EEnvironmentState::MAX);
			CurrentEquipmentInstance = SpawnedItem;
			LOGINVEN(Warning, TEXT("ItemToEquip Name: %s, Amount %d"), *ItemData.Name.ToString(), ItemData.Amount);
			SetEquipInfo(SlotIndex, SpawnedItem);
			
			if (UEquipUseComponent* EquipComp = CachedDiver->GetEquipUseComponent())
			{
				if (GetCurrentEquipmentItemData())
				{
					EquipComp->Initialize(*GetCurrentEquipmentItemData());
					bIsWeapon = EquipComp->bIsWeapon;
					EquipmentType = EquipComp->GetEquipType();
					bHasNoAnimation = EquipComp->bHasNoAnimation;
				}
			}
			CurrentEquipItem = SpawnedItem;
			OnRep_CurrentEquipItem();
		}
	}
	if (AUnderwaterCharacter* UnderwaterCharacter = Cast<AUnderwaterCharacter>(Pawn))
	{
		LOGINVEN(Log, TEXT("Play Equip Montage!!"));
		if (UnderwaterCharacter->SpawnedTool)
		{
			UnderwaterCharacter->CleanupToolAndEffects();
		}
		if (!bHasNoAnimation)
		{
			PlayEquipAnimation(UnderwaterCharacter, EquipmentType);
		}
	}
}

void UADInventoryComponent::UnEquip()
{
	// EquipComp의 장비 현재값 초기화
	if (CurrentEquipmentInstance)
	{
		if (APlayerState* PS = Cast<APlayerState>(GetOwner()))
		{
			if (APawn* Pawn = PS->GetPawn())
			{
				AUnderwaterCharacter* Diver = Cast<AUnderwaterCharacter>(Pawn);
				if (!Diver)
					return;

				const float MontageStopSeconds = 1.0f;
				Diver->M_StopAllMontagesOnBothMesh(MontageStopSeconds);
				if (UEquipUseComponent* EquipComp = Diver->GetEquipUseComponent())
				{
					EquipComp->DeinitializeEquip();
					InventoryList.MarkItemDirty(InventoryList.Items[FindItemIndexByName(CurrentEquipmentInstance->ItemData.Name)]);
				}
			
			}
		}
		C_InventoryPlaySound(ESFX::UnEquip);
		/*LOGINVEN(Warning, TEXT("UnEquipItem %s"), *CurrentEquipmentInstance->ItemData.Name.ToString());
		if(CurrentEquipmentInstance)
			CurrentEquipmentInstance->Destroy();*/
		if (CachedDiver)
		{
			if (UEquipRenderComponent* EquipRender = CachedDiver->GetEquipRenderComponent())
			{
				EquipRender->DetachItem(CurrentEquipmentInstance);
			}
			CurrentEquipmentInstance->Destroy();
			CurrentEquipItem->Destroy();
			CurrentEquipmentInstance = nullptr;
			CurrentEquipItem = nullptr;
		}
	}
	SetEquipInfo(INDEX_NONE, nullptr);

}

void UADInventoryComponent::DropItem(FItemData& ItemData)
{
	// EquipComp의 장비 현재값 초기화
	APlayerState* PS = Cast<APlayerState>(GetOwner());
	if (!PS) return;
	APawn* Pawn = PS->GetPawn();
	if (!Pawn) return;

	if (CurrentEquipmentInstance && ItemData.Name == CurrentEquipmentInstance->ItemData.Name)
	{
		AUnderwaterCharacter* Diver = Cast<AUnderwaterCharacter>(Pawn);
		if (!Diver)
			return;

		const float MontageStopSeconds = 1.0f;
		Diver->M_StopAllMontagesOnBothMesh(MontageStopSeconds);
		if (UEquipUseComponent* EquipComp = Diver->GetEquipUseComponent())
		{
			EquipComp->DeinitializeEquip();
		}
	}
	FVector DropLocation = GetDropLocation();
	AADUseItem* SpawnItem = GetWorld()->SpawnActor<AADUseItem>(AADUseItem::StaticClass(), DropLocation, FRotator::ZeroRotator);
	AUnderwaterCharacter* UnderwaterCharacter = Cast<AUnderwaterCharacter>(Pawn);
	EEnvironmentState CurrentEnviromnent = UnderwaterCharacter->GetEnvironmentState();
	if (CurrentEnviromnent == EEnvironmentState::Underwater)
	{
		M_SpawnItemEffect(ESFX::DropItem, DropItemEffect, DropLocation);
	}
	SpawnItem->M_SetItemVisible(true);
	SpawnItem->SetItemInfo(ItemData, false, UnderwaterCharacter->GetEnvironmentState());
	LOGINVEN(Warning, TEXT("Spawn Item To Drop : %s"), *ItemData.Name.ToString());
}

void UADInventoryComponent::OnInventoryInfoUpdate(int32 MassInfo, int32 PriceInfo)
{
	TotalWeight = FMath::Max(TotalWeight+MassInfo, 0);
	TotalPrice = FMath::Max(TotalPrice+PriceInfo, 0);
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

void UADInventoryComponent::TryCachedDiver()
{
	if (CachedDiver) return;

	if (AADPlayerState* PlayerState = Cast<AADPlayerState>(GetOwner()))
	{
		if (APawn* Pawn = PlayerState->GetPawn())
		{
			if (AUnderwaterCharacter* Diver = Cast<AUnderwaterCharacter>(Pawn))
			{
				CachedDiver = Diver;
				SetComponentTickEnabled(false);

				if (CurrentEquipItem)
				{
					OnRep_CurrentEquipItem();
				}
			}
		}
	}
}

FGameplayTagContainer UADInventoryComponent::MakeItemTags(const FName ItemName) const
{
	FGameplayTagContainer Tags;

	if (!ItemName.IsNone())
	{
		const FGameplayTag IdTag = RequestTag(TEXT("Item.id"), ItemName.ToString());
		if (IdTag.IsValid()) Tags.AddTag(IdTag);

	}

	return Tags;
}



void UADInventoryComponent::SetChargeBatteryInstance(UChargeBatteryWidget* BatteryWidget)
{
	ChargeBatteryWidget = BatteryWidget;
	CheckItemsForBattery();
	LOGINVEN(Warning, TEXT("Allocate BatteryWidget To Inventory"));
}

USoundSubsystem* UADInventoryComponent::GetSoundSubsystem()
{
	if (SoundSubsystem)
	{
		return SoundSubsystem;
	}

	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		SoundSubsystem = GI->GetSubsystem<USoundSubsystem>();
		return SoundSubsystem;
	}
	return nullptr;
}

UAnimMontage* UADInventoryComponent::GetDrawMontageByType(EEquipmentType InType) const
{
	switch (InType)
	{
	case EEquipmentType::HarpoonGun: return HarpoonDrawMontage;
	case EEquipmentType::FlareGun:   return HarpoonDrawMontage;
	case EEquipmentType::Shotgun:    return HarpoonDrawMontage;
	case EEquipmentType::DPV:        return DPVDrawMontage;
	case EEquipmentType::Mine:       return DPVDrawMontage;
	case EEquipmentType::ToyHammer:  return HammerDrawMontage;  
	default:                         return nullptr;
	}
}

UMissionEventHubComponent* UADInventoryComponent::GetMissionHub()
{
	if (CachedHub) return CachedHub;
	if (AGameStateBase* GS = UGameplayStatics::GetGameState(this))
		CachedHub = GS->FindComponentByClass<UMissionEventHubComponent>();
	return CachedHub;
}

