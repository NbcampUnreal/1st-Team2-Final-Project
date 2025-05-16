﻿#include "Shops/Shop.h"

#include "Character/UnitBase.h"
#include "Character/UnderwaterCharacter.h"
#include "Character/UpgradeComponent.h"

#include "Shops/ShopWidgets/ShopCategoryTabWidget.h"
#include "Shops/ShopWidgets/ShopWidget.h"
#include "Shops/ShopItemEntryData.h"
#include "Shops/ShopWidgets/ShopElementInfoWidget.h"
#include "Shops/ShopWidgets/ShopItemSlotWidget.h"
#include "ShopInteractionComponent.h"

#include "AbyssDiverUnderWorld.h"
#include "Framework/ADPlayerState.h"
#include "Inventory/ADInventoryComponent.h"
#include "Subsystems/DataTableSubsystem.h"

#include "DataRow/UpgradeDataRow.h"
#include "DataRow/FADItemDataRow.h"


#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

#pragma region FShopItemId

void FShopItemId::PostReplicatedAdd(const FShopItemIdList& InArraySerializer)
{
	UE_LOG(LogTemp, Log, TEXT("Shop item added: Id = %d"), Id);

	int32 Index = InArraySerializer.IdList.IndexOfByKey(*this);
	FShopItemListChangeInfo Info(InArraySerializer.TabType, Index, Id, EShopItemChangeType::Added);

	InArraySerializer.OnShopItemListChangedDelegate.Broadcast(Info);
}

void FShopItemId::PostReplicatedChange(const FShopItemIdList& InArraySerializer)
{
	UE_LOG(LogTemp, Log, TEXT("Shop item changed: Id = %d"), Id);

	int32 Index = InArraySerializer.IdList.IndexOfByKey(*this);
	FShopItemListChangeInfo Info(InArraySerializer.TabType, Index, Id, EShopItemChangeType::Modified);

	InArraySerializer.OnShopItemListChangedDelegate.Broadcast(Info);
}

void FShopItemId::PreReplicatedRemove(const FShopItemIdList& InArraySerializer)
{
	UE_LOG(LogTemp, Log, TEXT("Shop item removed: Id = %d"), Id);

	int32 Index = InArraySerializer.IdList.IndexOfByKey(*this);
	FShopItemListChangeInfo Info(InArraySerializer.TabType, Index, Id, EShopItemChangeType::Removed);

	InArraySerializer.OnShopItemListChangedDelegate.Broadcast(Info);
}

#pragma endregion

#pragma region FShopItemIdList

bool FShopItemIdList::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FShopItemId, FShopItemIdList>(IdList, DeltaParams, *this);
}

int32 FShopItemIdList::Contains(uint8 CompareId)
{
	int32 ElementCount = IdList.Num();
	for (int32 i = 0; i < ElementCount; ++i)
	{
		if (IdList[i].Id == CompareId)
		{
			return i;
		}
	}

	return INDEX_NONE;
}

bool FShopItemIdList::TryAdd(uint8 NewId)
{
	if (Contains(NewId) != INDEX_NONE)
	{
		UE_LOG(LogTemp, Log, TEXT("You Trying to Add Exist Id : %d"), NewId);
		return false;
	}

	FShopItemId ShopId;
	ShopId.Id = NewId;

	IdList.Add(ShopId);
	MarkItemDirty(IdList.Last());

	return true;
}

int32 FShopItemIdList::Remove(uint8 Id)
{
	int32 Index = Contains(Id);
	if (Index == INDEX_NONE)
	{
		UE_LOG(LogTemp, Log, TEXT("You Trying to Remove Not Exist Id : %d"), Id);
		return INDEX_NONE;
	}

	IdList.RemoveAt(Index);
	MarkArrayDirty();
	return Index;
}

void FShopItemIdList::Modify(uint8 InIndex, uint8 NewId)
{
	if (IdList.IsValidIndex(InIndex) == false)
	{
		UE_LOG(LogTemp, Log, TEXT("You Trying to Modify From Not Valid Index : %d"), InIndex);
		return;
	}

	FShopItemId& ItemId = IdList[InIndex];
	ItemId.Id = NewId;
	MarkItemDirty(ItemId);
}

uint8 FShopItemIdList::GetId(uint8 InIndex) const
{
	if (IdList.IsValidIndex(InIndex) == false)
	{
		UE_LOG(LogTemp, Log, TEXT("You Trying to Get From Not Valid Index"));
		return INDEX_NONE;
	}

	return IdList[InIndex].Id;
}
#pragma endregion

AShop::AShop()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	ShopMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShopMesh"));
	SetRootComponent(ShopMeshComponent);

	ItemMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh"));
	ItemMeshComponent->SetupAttachment(RootComponent);
	ItemMeshComponent->SetVisibleInSceneCaptureOnly(true);
	ItemMeshComponent->SetIsReplicated(false);
	ItemMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InteractableComp = CreateDefaultSubobject<UADInteractableComponent>(TEXT("InteractableComp"));

	bIsOpened = false;
	CurrentSelectedUpgradeType = EUpgradeType::Max;
}

void AShop::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShop, ShopConsumableItemIdList);
	DOREPLIFETIME(AShop, ShopEquipmentItemIdList);
}

void AShop::BeginPlay()
{
	Super::BeginPlay();

	InitShopWidget();
	InitData();
}

void AShop::Interact_Implementation(AActor* InstigatorActor)
{
	if (HasAuthority() == false)
	{
		return;
	}

	ACharacter* InteractingCharacter = Cast<ACharacter>(InstigatorActor);
	if (InteractingCharacter == nullptr)
	{
		return;
	}

	// 캐릭터로부터 컴포넌트 Get, 나중에 Getter로 가져옴
	UShopInteractionComponent* ShopInteractionComp = InteractingCharacter->FindComponentByClass<UShopInteractionComponent>();
	if (ShopInteractionComp == nullptr)
	{
		LOGV(Warning, TEXT("ShopInteractionComp == nullptr"));
		return;
	}

	ShopInteractionComp->SetCurrentInteractingShop(this);
	ShopInteractionComp->C_OpenShop(this);
}

void AShop::OpenShop(AUnderwaterCharacter* Requester)
{
	if (Requester->IsLocallyControlled() == false)
	{
		return;
	}

	if (bIsOpened)
	{
		return;
	}

	InitUpgradeView();
	ShopWidget->AddToViewport();

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	PC->SetInputMode(FInputModeUIOnly());
	PC->SetShowMouseCursor(true);
	bIsOpened = true;
	PC->SetIgnoreMoveInput(true);
}

void AShop::CloseShop(AUnderwaterCharacter* Requester)
{
	if (Requester->IsLocallyControlled() == false)
	{
		return;
	}

	if (bIsOpened == false)
	{
		return;
	}

	ShopWidget->RemoveFromParent();

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	PC->SetInputMode(FInputModeGameOnly());
	PC->SetShowMouseCursor(false);
	bIsOpened = false;
	PC->SetIgnoreMoveInput(false);
}

EBuyResult AShop::BuyItem(uint8 ItemId, uint8 Quantity, AUnderwaterCharacter* Buyer)
{
	if (HasAuthority() == false)
	{
		LOGVN(Error, TEXT("Has No Authority"));
		return EBuyResult::HasNoAuthority;
	}

	/*if (돈이 없으면)
	{
		return EBuyResult::NotEnoughMoney;
	}*/

	if (HasItem(ItemId) == false)
	{
		return EBuyResult::NotExistItem;
	}
	
	AADPlayerState* PS = Cast<AADPlayerState>(Buyer->GetPlayerState());
	if (PS == nullptr)
	{
		LOGV(Error, TEXT("PS == nullptr"));
		return EBuyResult::FailedFromOtherReason;
	}

	FFADItemDataRow* ItemDataRow = GetGameInstance()->GetSubsystem<UDataTableSubsystem>()->GetItemData(ItemId);
	if (ItemDataRow == nullptr)
	{
		LOGV(Error, TEXT("ItemData == nullptr"));
		return EBuyResult::FailedFromOtherReason;
	}

	FItemData ItemData;
	ItemData.Amount = ItemDataRow->Amount;
	ItemData.Id = ItemDataRow->Id;
	ItemData.ItemType = ItemDataRow->ItemType;
	ItemData.Mass = ItemDataRow->Weight;
	ItemData.Name = ItemDataRow->Name;
	ItemData.Price = ItemDataRow->Price;
	ItemData.Quantity = Quantity;
	ItemData.Thumbnail = ItemDataRow->Thumbnail;

	PS->GetInventory()->AddInventoryItem(ItemData);

	// 돈 차감 로직
	// 인벤토리에 아이템 넣기
	LOGV(Log, TEXT("Buying Item Succeeded : %s"), *ItemDataRow->Name.ToString());
	return EBuyResult::Succeeded;
}

ESellResult AShop::SellItem(uint8 ItemId, AUnderwaterCharacter* Seller)
{
	if (HasAuthority() == false)
	{
		LOGVN(Log, TEXT("Has No Authority"));
		return ESellResult::HasNoAuthority;
	}

	/*if (아이템 안 갖고 있으면)
	{
		return ESellResult::NotExistItem;
	}*/

	// 돈 추가
	// 인벤토리에서 아이템 삭제

	return ESellResult::Succeeded;
}

void AShop::AddItems(const TArray<uint8>& Ids, EShopCategoryTab TabType)
{
	if (HasAuthority() == false)
	{
		LOGVN(Log, TEXT("Has No Authority"));
		return;
	}

	for (auto& Id : Ids)
	{
		AddItemToList(Id, TabType);
	}
}

void AShop::AddItemToList(uint8 ItemId, EShopCategoryTab TabType)
{
	if (HasAuthority() == false)
	{
		LOGVN(Log, TEXT("Has No Authority"));
		return;
	}

	if (TabType >= EShopCategoryTab::Max)
	{
		LOGVN(Error, TEXT("Weird Category Type : %d"), TabType);
		return;
	}

	bool bIsAddSucceeded = false;
	int32 SlotIndex;
	switch (TabType)
	{
	case EShopCategoryTab::Consumable:
		bIsAddSucceeded = ShopConsumableItemIdList.TryAdd(ItemId);
		SlotIndex = ShopConsumableItemIdList.IdList.Num() - 1;
		break;
	case EShopCategoryTab::Equipment:
		bIsAddSucceeded = ShopEquipmentItemIdList.TryAdd(ItemId);
		SlotIndex = ShopEquipmentItemIdList.IdList.Num() - 1;
		break;
	case EShopCategoryTab::Upgrade:
		LOGV(Error, TEXT("Can't Add Item to Upgrade Tab"));
		return;
	case EShopCategoryTab::Max:
		check(false);
		return;
	default:
		check(false);
		return;
	}

	if (bIsAddSucceeded == false)
	{
		return;
	}

	FFADItemDataRow* ItemDataRow = GetGameInstance()->GetSubsystem<UDataTableSubsystem>()->GetItemData(ItemId);
	if (ItemDataRow == nullptr)
	{
		LOGV(Error, TEXT("ItemData == nullptr"));
		return;
	}

	UShopItemEntryData* EntryData = NewObject<UShopItemEntryData>();
	EntryData->Init(SlotIndex, ItemDataRow->Thumbnail, ItemDataRow->Description); // 임시
	EntryData->OnEntryUpdatedFromDataDelegate.AddUObject(this, &AShop::OnSlotEntryWidgetUpdated);

	ShopWidget->AddItem(EntryData, TabType);

	if (TabType == EShopCategoryTab::Consumable)
	{
		LOGVN(Log, TEXT("ItemAdded to Consumable Tab, Index : %d"), SlotIndex);
	}
	else
	{
		LOGVN(Log, TEXT("ItemAdded to Equipment Tab, Index : %d"), SlotIndex);
	}
	
}

void AShop::RemoveItemToList(uint8 ItemId, EShopCategoryTab TabType)
{
	if (HasAuthority() == false)
	{
		LOGVN(Log, TEXT("Has No Authority"));
		return;
	}

	if (TabType >= EShopCategoryTab::Max)
	{
		LOGVN(Error, TEXT("Weird Category Type : %d"), TabType);
		return;
	}

	int32 RemovedIndex = INDEX_NONE;

	switch (TabType)
	{
	case EShopCategoryTab::Consumable:
		RemovedIndex = ShopConsumableItemIdList.Remove(ItemId);
		break;
	case EShopCategoryTab::Equipment:
		RemovedIndex = ShopEquipmentItemIdList.Remove(ItemId);
		break;
	case EShopCategoryTab::Upgrade:
		LOGV(Error, TEXT("Can't Remove Item From Upgrade Tab"));
		break;
	case EShopCategoryTab::Max:
		check(false);
		break;
	default:
		check(false);
		break;
	}

	if (RemovedIndex == INDEX_NONE)
	{
		return;
	}

	ShopWidget->RemoveItem(RemovedIndex, TabType);
}

void AShop::InitUpgradeView()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	AADPlayerState* PS = PC->GetPlayerState<AADPlayerState>();
	if (PS == nullptr)
	{
		LOGV(Error, TEXT("PS == nullptr"));
		return;
	}

	// 나중에 Get으로 대체
	UUpgradeComponent* UpgradeComp = PS->GetUpgradeComp();
	if (UpgradeComp == nullptr)
	{
		LOGV(Error, TEXT("UpgradeComp == nullptr"));
		return;
	}

	UEnum* UpgradeTypeEnum = StaticEnum<EUpgradeType>();
	if (UpgradeTypeEnum == nullptr)
	{
		LOGV(Error, TEXT("Cant Find Enum"));
		return;
	}

	UDataTableSubsystem* DataTableSubsystem = GetGameInstance()->GetSubsystem<UDataTableSubsystem>();
	if (DataTableSubsystem == nullptr)
	{
		LOGV(Error, TEXT("DataTableSubsystem == nullptr"));
		return;
	}

	// 마지막 값은 항상 EUpgradeType_MAX이 자동생성된다고 함.(선언한 MAX를 말하는게 아님)
	int32 EnumCount = UpgradeTypeEnum->NumEnums() - 1;

	if (CachedUpgradeGradeMap.Num() != EnumCount)
	{
		CachedUpgradeGradeMap.Init(-1, EnumCount);
	}

	bool bHasChanged = false;

	TArray<TObjectPtr<UShopItemEntryData>>& UpgradeTabEntryDataList = ShopWidget->GetUpgradeTabEntryDataList();
	if (UpgradeTabEntryDataList.Num() <= EnumCount)
	{
		UpgradeTabEntryDataList.SetNum(EnumCount);
	}

	LOGV(Error, TEXT("Enumcount : %d"), EnumCount);
	for (int32 i = 0; i < EnumCount - 1; ++i) // -1은 MAX 제외
	{
		EUpgradeType UpgradeType = EUpgradeType(i);
		uint8 Grade = UpgradeComp->GetCurrentGrade(UpgradeType);

		if (Grade == 0)
		{
			LOGVN(Error, TEXT("Weird Grade Detected, Type : %d"), UpgradeType);
			return;
		}

		if (CachedUpgradeGradeMap[i] == Grade)
		{
			LOGV(Log, TEXT("UpgradeState : UpgradeType(%d) - Grade(%d)"), UpgradeType, Grade);
			continue;
		}
		LOGV(Log, TEXT("UpgradeState(Renewed) : UpgradeType(%d) - Grade(%d)"), UpgradeType, Grade);
		CachedUpgradeGradeMap[i] = Grade;

		FUpgradeDataRow* UpgradeDataRow = DataTableSubsystem->GetUpgradeData(UpgradeType, Grade);

		UShopItemEntryData* UpgradeEntryData = NewObject<UShopItemEntryData>();
		UpgradeEntryData->Init(i, nullptr, FString(TEXT("Temp Tooltip")));
		UpgradeEntryData->OnEntryUpdatedFromDataDelegate.AddUObject(this, &AShop::OnSlotEntryWidgetUpdated);
		UpgradeTabEntryDataList[i] = UpgradeEntryData;

		bHasChanged = true;
	}

	if (bHasChanged == false)
	{
		return;
	}

	if (ShopWidget->GetCurrentActivatedTab() != EShopCategoryTab::Upgrade)
	{
		return;
	}

	ShopWidget->RefreshItemView();
	OnUpgradeSlotEntryClicked(int32(CurrentSelectedUpgradeType));
}

void AShop::Interact_Test(AActor* InstigatorActor)
{
	InteractableComp->Interact(InstigatorActor);
}

void AShop::InitShopWidget()
{
	ShopConsumableItemIdList.IdList.Empty();
	ShopConsumableItemIdList.OnShopItemListChangedDelegate.AddUObject(this, &AShop::OnShopItemListChanged);
	ShopConsumableItemIdList.ShopOwner = this;
	ShopConsumableItemIdList.TabType = EShopCategoryTab::Consumable;

	ShopEquipmentItemIdList.IdList.Empty();
	ShopEquipmentItemIdList.OnShopItemListChangedDelegate.AddUObject(this, &AShop::OnShopItemListChanged);
	ShopEquipmentItemIdList.ShopOwner = this;
	ShopEquipmentItemIdList.TabType = EShopCategoryTab::Equipment;

	ShopWidget = CreateWidget<UShopWidget>(GetWorld(), ShopWidgetClass, FName(TEXT("ShopWidget")));
	check(ShopWidget);

	ShopWidget->SetCurrentActivatedTab(EShopCategoryTab::Consumable);
	ShopWidget->OnShopCloseButtonClickedDelegate.AddUObject(this, &AShop::OnCloseButtonClicked);

	UShopElementInfoWidget* InfoWidget = ShopWidget->GetInfoWidget();
	check(InfoWidget);

	InfoWidget->Init(ItemMeshComponent);
	InfoWidget->OnBuyButtonClickedDelegate.AddUObject(this, &AShop::OnBuyButtonClicked);
}

void AShop::InitData()
{
	if (HasAuthority() == false)
	{
		return;
	}

	ShopConsumableItemIdList.MarkArrayDirty();

	for (const auto& Id : DefaultConsumableItemIdList)
	{
		AddItemToList(Id, EShopCategoryTab::Consumable);
	}

	ShopWidget->ShowItemViewForTab(EShopCategoryTab::Consumable);

	ShopEquipmentItemIdList.MarkArrayDirty();

	for (const auto& Id : DefaultEquipmentItemIdList)
	{
		AddItemToList(Id, EShopCategoryTab::Equipment);
	}
}

void AShop::OnShopItemListChanged(const FShopItemListChangeInfo& Info)
{
	if (Info.ChangeType >= EShopItemChangeType::Max)
	{
		LOGVN(Error, TEXT("Weird Change Type : %d"), Info.ChangeType);
		return;
	}

	if (Info.ShopTab >= EShopCategoryTab::Upgrade)
	{
		return;
	}

	UShopItemEntryData* EntryData = nullptr;
	FFADItemDataRow* ItemDataRow = GetGameInstance()->GetSubsystem<UDataTableSubsystem>()->GetItemData(Info.ItemIdAfter);
	if (ItemDataRow == nullptr)
	{
		LOGV(Error, TEXT("ItemData == nullptr"));
		return;
	}

	switch (Info.ChangeType)
	{
	case EShopItemChangeType::Added:

		EntryData = NewObject<UShopItemEntryData>();
		EntryData->Init(Info.ShopIndex, ItemDataRow->Thumbnail, ItemDataRow->Description); // 임시
		EntryData->OnEntryUpdatedFromDataDelegate.AddUObject(this, &AShop::OnSlotEntryWidgetUpdated);
		
		ShopWidget->AddItem(EntryData, Info.ShopTab);
		LOGVN(Log, TEXT("Add Data End, Category : %d"), ItemDataRow->ItemType);
		break;
	case EShopItemChangeType::Removed:
		ShopWidget->RemoveItem(Info.ShopIndex, Info.ShopTab);
		break;
	case EShopItemChangeType::Modified:
		ShopWidget->ModifyItem(Info.ShopIndex, ItemDataRow->Thumbnail, ItemDataRow->Description, Info.ShopTab);
		break;
	case EShopItemChangeType::Max:
		check(false);
		break;
	default:
		check(false);
		break;
	}

	ShopWidget->RefreshItemView();
}

void AShop::OnSlotEntryWidgetUpdated(UShopItemSlotWidget* SlotEntryWidget)
{
	SlotEntryWidget->OnShopItemSlotWidgetClickedDelegate.BindUObject(this, &AShop::OnSlotEntryClicked);
}

void AShop::OnSlotEntryClicked(int32 ClickedSlotIndex)
{
	EShopCategoryTab CurrentTab = ShopWidget->GetCurrentActivatedTab();

	if (CurrentTab >= EShopCategoryTab::Max)
	{
		LOGV(Error, TEXT("Weird Tab Type : %d"), CurrentTab);
		return;
	}

	int32 ItemId;

	switch (CurrentTab)
	{
	case EShopCategoryTab::Consumable:
		ItemId = ShopConsumableItemIdList.IdList[ClickedSlotIndex].Id;
		break;
	case EShopCategoryTab::Equipment:
		ItemId = ShopEquipmentItemIdList.IdList[ClickedSlotIndex].Id;
		break;
	case EShopCategoryTab::Upgrade:
		LOGV(Error, TEXT("UpgradeSlot Click!, Index : %d"), ClickedSlotIndex);
		OnUpgradeSlotEntryClicked(ClickedSlotIndex);
		return;
	case EShopCategoryTab::Max:
		check(false);
		return;
	default:
		check(false);
		return;
	}

	FFADItemDataRow* ItemDataRow = GetGameInstance()->GetSubsystem<UDataTableSubsystem>()->GetItemData(ItemId);
	if (ItemDataRow == nullptr)
	{
		LOGV(Error, TEXT("ItemData == nullptr"));
		return;
	}

	USkeletalMesh* ItemMesh = ItemDataRow->SkeletalMesh;

	ShopWidget->ShowItemInfos(ItemMesh, ItemDataRow->Description, ItemDataRow->Description);
	LOGV(Log, TEXT("Showing Item Infos..., id : %d"), ItemId);
	CurrentSelectedItemId = ItemId;
}

void AShop::OnBuyButtonClicked(int32 Quantity)
{
	EShopCategoryTab CurrentTab = ShopWidget->GetCurrentActivatedTab();

	if (CurrentTab == EShopCategoryTab::Upgrade)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		AADPlayerState* PS = PC->GetPlayerState<AADPlayerState>();
		if (PS == nullptr)
		{
			LOGV(Error, TEXT("PS == nullptr"));
			return;
		}

		UUpgradeComponent* UpgradeComp = PS->GetUpgradeComp();
		if (UpgradeComp == nullptr)
		{
			LOGV(Error, TEXT("UpgradeComp == nullptr"));
			return;
		}

		bool bIsMaxGrade = UpgradeComp->IsMaxGrade(CurrentSelectedUpgradeType);
		if (bIsMaxGrade)
		{
			LOGV(Log, TEXT("This Upgrade Type Has Reached to Max Level"));
			return;
		}

		UpgradeComp->S_RequestUpgrade(CurrentSelectedUpgradeType);
		return;
	}

	AUnderwaterCharacter* BuyingCharacter = Cast<AUnderwaterCharacter>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetPawn());
	if (BuyingCharacter == nullptr)
	{
		LOGV(Warning, TEXT("BuyingCharacter == nullptr"));
		return;
	}

	UShopInteractionComponent* ShopInteractionComp = BuyingCharacter->GetShopInteractionComponent();
	if (ShopInteractionComp == nullptr)
	{
		LOGV(Warning, TEXT("ShopInteractionComp == nullptr"));
		return;
	}

	ShopInteractionComp->S_RequestBuyItem(CurrentSelectedItemId, Quantity);
}

void AShop::OnCloseButtonClicked()
{
	AUnderwaterCharacter* Requester = Cast<AUnderwaterCharacter>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetPawn());
	if (Requester == nullptr)
	{
		LOGVN(Error, TEXT("Requester == nullptr"));
		return;
	}

	CloseShop(Requester);
}

void AShop::OnUpgradeSlotEntryClicked(int32 ClickedSlotIndex)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	AADPlayerState* PS = PC->GetPlayerState<AADPlayerState>();
	if (PS == nullptr)
	{
		LOGV(Error, TEXT("PS == nullptr"));
		return;
	}

	UUpgradeComponent* UpgradeComp = PS->GetUpgradeComp();
	if (UpgradeComp == nullptr)
	{
		LOGV(Error, TEXT("UpgradeComp == nullptr"));
		return;
	}

	EUpgradeType UpgradeType = EUpgradeType(ClickedSlotIndex);
	
	uint8 CurrentGrade = UpgradeComp->GetCurrentGrade(UpgradeType);
	if (CurrentGrade == 0)
	{
		LOGV(Log, TEXT("Weird Upgrade Type Detected : %d"), UpgradeType);
		return;
	}

	UDataTableSubsystem* DataTableSubSystem = GetGameInstance()->GetSubsystem<UDataTableSubsystem>();
	if (DataTableSubSystem == nullptr)
	{
		LOGV(Error, TEXT("DataTableSubSystem == nullptr"));
		return;
	}

	bool bIsMaxGrade = UpgradeComp->IsMaxGrade(UpgradeType);

	FUpgradeDataRow* UpgradeDataRow = DataTableSubSystem->GetUpgradeData(UpgradeType, CurrentGrade);
	int32 Price = bIsMaxGrade ? 0 : DataTableSubSystem->GetUpgradeData(UpgradeType, CurrentGrade + 1)->Price;
	ShopWidget->ShowUpgradeInfos(nullptr, CurrentGrade, bIsMaxGrade, Price, TEXT("임시 텍스트"));
	CurrentSelectedUpgradeType = UpgradeType;

	LOGV(Log, TEXT("UpgradeViewSlotClicked, Index : %d"), ClickedSlotIndex);
}

bool AShop::HasItem(int32 ItemId)
{
	bool bHasItem = (ShopConsumableItemIdList.Contains(ItemId) != INDEX_NONE);
	bHasItem = bHasItem || (ShopEquipmentItemIdList.Contains(ItemId) != INDEX_NONE);

	return bHasItem;
}

bool AShop::IsOpened() const
{
	return bIsOpened;
}

UADInteractableComponent* AShop::GetInteractableComponent() const
{
	return InteractableComp;
}
