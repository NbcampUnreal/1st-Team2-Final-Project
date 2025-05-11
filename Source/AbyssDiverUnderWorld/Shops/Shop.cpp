#include "Shops/Shop.h"

#include "Character/UnitBase.h"
#include "Shops/ShopWidgets/ShopCategoryTabWidget.h"
#include "Shops/ShopWidgets/ShopWidget.h"
#include "Shops/ShopItemEntryData.h"
#include "Shops/ShopWidgets/ShopElementInfoWidget.h"
#include "Shops/ShopWidgets/ShopItemSlotWidget.h"
#include "AbyssDiverUnderWorld.h"

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

	ItemMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMeshComponent->SetupAttachment(RootComponent);
	ItemMeshComponent->SetVisibleInSceneCaptureOnly(true);
	ItemMeshComponent->SetIsReplicated(false);
	ItemMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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

void AShop::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

EBuyResult AShop::BuyItem(uint8 ItemId, EShopCategoryTab TabType)
{
	if (HasAuthority() == false)
	{
		LOGVN(Log, TEXT("Has No Authority"));
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

	// 돈 차감 로직
	// 인벤토리에 아이템 넣기
	return EBuyResult::Succeeded;
}

ESellResult AShop::SellItem(uint8 ItemId, class AUnitBase* Seller)
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
		LOGV(Error, TEXT("Upgrade Tab is not Supported Currently"));
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

	FFADItemDataRow*& Data = DataTableArray[ItemId];

	UShopItemEntryData* EntryData = NewObject<UShopItemEntryData>();
	EntryData->Init(SlotIndex, Data->Thumbnail, Data->Description); // 임시
	EntryData->OnEntryUpdatedFromDataDelegate.AddUObject(this, &AShop::OnSlotEntryWidgetUpdated);

	ShopWidget->AddItem(EntryData, TabType);

	if (TabType == EShopCategoryTab::Consumable)
	{
		LOGVN(Error, TEXT("ItemAdded to Consumable Tab, Index : %d"), SlotIndex);
	}
	else
	{
		LOGVN(Error, TEXT("ItemAdded to Equipment Tab, Index : %d"), SlotIndex);
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
		LOGV(Error, TEXT("Upgrade Tab is not Supported Currently"));
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

void AShop::InitShopWidget()
{
	ShopConsumableItemIdList.IdList.Empty();
	ShopConsumableItemIdList.OnShopItemListChangedDelegate.AddUObject(this, &AShop::OnShopItemListChanged);
	ShopConsumableItemIdList.ShopOwner = this;
	ShopConsumableItemIdList.TabType = EShopCategoryTab::Consumable;

	ShopEquipmentItemIdList.IdList.Empty();
	ShopEquipmentItemIdList.OnShopItemListChangedDelegate.AddUObject(this, &AShop::OnShopItemListChanged);
	ShopEquipmentItemIdList.ShopOwner = this;
	ShopConsumableItemIdList.TabType = EShopCategoryTab::Consumable;

	ShopWidget = CreateWidget<UShopWidget>(GetWorld(), ShopWidgetClass, FName(TEXT("ShopWidget")));
	check(ShopWidget);

	ShopWidget->SetCurrentActivatedTab(EShopCategoryTab::Consumable);
	ShopWidget->AddToViewport();

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	PC->SetInputMode(FInputModeUIOnly());
	PC->SetShowMouseCursor(true);

	ShopWidget->GetInfoWidget()->Init(ItemMeshComponent);
}

void AShop::InitData()
{
	// 아이템 데이터 테이블로부터 아이템 Id 리스트를 통해 데이터 가져와야 함.
	// 현재 임시 테이블 사용
	ItemDataTable->GetAllRows<FFADItemDataRow>(TEXT("TestShopItemData"), DataTableArray);

	Algo::Sort(DataTableArray, [](const FFADItemDataRow* A, const FFADItemDataRow* B)
		{
			return A->Id < B->Id;
		});

	ItemMeshDataTable->GetAllRows<FItemMeshDataRow>(TEXT("TestMeshItemData"), MeshDataTableArray);

	Algo::Sort(MeshDataTableArray, [](const FItemMeshDataRow* A, const FItemMeshDataRow* B)
		{
			return A->ItemId < B->ItemId;
		});

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
	LOGVN(Error, TEXT("Changed!"));
	
	if (Info.ChangeType >= EShopItemChangeType::Max)
	{
		LOGVN(Error, TEXT("Weird Change Type : %d"), Info.ChangeType);
		return;
	}

	UShopItemEntryData* EntryData = nullptr;
	FFADItemDataRow*& Data = DataTableArray[Info.ItemIdAfter];

	switch (Info.ChangeType)
	{
	case EShopItemChangeType::Added:

		EntryData = NewObject<UShopItemEntryData>();
		EntryData->Init(Info.ShopIndex, Data->Thumbnail, Data->Description); // 임시
		EntryData->OnEntryUpdatedFromDataDelegate.AddUObject(this, &AShop::OnSlotEntryWidgetUpdated);

		if (Data->ItemType == EItemType::Consumable)
		{
			ShopWidget->AddItem(EntryData, EShopCategoryTab::Consumable);
		}
		else if (Data->ItemType == EItemType::Equipment)
		{
			ShopWidget->AddItem(EntryData, EShopCategoryTab::Equipment);
		}

		break;
	case EShopItemChangeType::Removed:
		ShopWidget->RemoveItem(Info.ShopIndex, Info.ShopTab);
		break;
	case EShopItemChangeType::Modified:
		ShopWidget->ModifyItem(Info.ShopIndex, Data->Thumbnail, Data->Description, Info.ShopTab);
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
	LOG(TEXT("%s bind Finc"), *SlotEntryWidget->GetName());
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
		LOGV(Error, TEXT("Upgrade is Not Supported Tab Type"));
		return;
	case EShopCategoryTab::Max:
		check(false);
		return;
	default:
		check(false);
		return;
	}

	UStaticMesh* ItemMesh = MeshDataTableArray[ItemId]->ItemMesh;

	ShopWidget->ShowItemInfos(ItemMesh, DataTableArray[ItemId]->Description, DataTableArray[ItemId]->Description);
	LOG(TEXT("Showing Item Infos..., id : %d"), ItemId);
}

bool AShop::HasItem(int32 ItemId)
{
	return (ShopConsumableItemIdList.Contains(ItemId) != INDEX_NONE);
}

bool AShop::IsItemMeshCached(int32 ItemId)
{
	return CachedMeshList.Contains(ItemId);
}