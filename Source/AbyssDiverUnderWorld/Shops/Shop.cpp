#include "Shops/Shop.h"

#include "Character/UnitBase.h"
#include "Shops/ShopWidgets/ShopCategoryTabWidget.h"
#include "Shops/ShopWidgets/ShopWidget.h"
#include "Shops/ShopItemEntryData.h"
#include "DataRow/FADItemDataRow.h"
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

void FShopItemIdList::Remove(uint8 Id)
{
	int32 Index = Contains(Id);
	if (Index == INDEX_NONE)
	{
		UE_LOG(LogTemp, Log, TEXT("You Trying to Remove Not Exist Id : %d"), Id);
		return;
	}

	IdList.RemoveAt(Index);
	MarkArrayDirty();
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

	/*if (���� ������)
	{
		return EBuyResult::NotEnoughMoney;
	}*/

	if (HasItem(ItemId) == false)
	{
		return EBuyResult::NotExistItem;
	}

	// �� ���� ����
	// �κ��丮�� ������ �ֱ�
	return EBuyResult::Succeeded;
}

ESellResult AShop::SellItem(uint8 ItemId, class AUnitBase* Seller)
{
	if (HasAuthority() == false)
	{
		LOGVN(Log, TEXT("Has No Authority"));
		return ESellResult::HasNoAuthority;
	}

	/*if (������ �� ���� ������)
	{
		return ESellResult::NotExistItem;
	}*/

	// �� �߰�
	// �κ��丮���� ������ ����

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

	ShopConsumableItemIdList.TryAdd(ItemId);
}

void AShop::RemoveItemToList(uint8 ItemId, EShopCategoryTab TabType)
{
	if (HasAuthority() == false)
	{
		LOGVN(Log, TEXT("Has No Authority"));
		return;
	}

	ShopConsumableItemIdList.Remove(ItemId);
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
}

void AShop::InitData()
{
	// ������ ������ ���̺�κ��� ������ Id ����Ʈ�� ���� ������ �����;� ��.
	// ���� �ӽ� ���̺� ���
	ItemDataTable->GetAllRows<FFADItemDataRow>(TEXT("TestShopItemData"), DataTableArray);

	Algo::Sort(DataTableArray, [](const FFADItemDataRow* A, const FFADItemDataRow* B)
		{
			return A->Id < B->Id;
		});

	if (HasAuthority() == false)
	{
		return;
	}

	ShopConsumableItemIdList.MarkArrayDirty();

	for (const auto& Id : DefaultConsumableItemIdList)
	{
		FFADItemDataRow*& Data = DataTableArray[Id];

		UShopItemEntryData* EntryData = NewObject<UShopItemEntryData>();
		EntryData->Init(Data->Price, Data->Thumbnail, Data->Description); // �ӽ�

		if (Data->ItemType == EItemType::Consumable)
		{
			ShopWidget->AddItem(EntryData, EShopCategoryTab::Consumable);
		}

		ShopConsumableItemIdList.TryAdd(Id);
	}

	ShopWidget->ShowItemViewForTab(EShopCategoryTab::Consumable);

	ShopEquipmentItemIdList.MarkArrayDirty();

	for (const auto& Id : DefaultEquipmentItemIdList)
	{
		FFADItemDataRow*& Data = DataTableArray[Id];

		UShopItemEntryData* EntryData = NewObject<UShopItemEntryData>();
		EntryData->Init(Data->Price, Data->Thumbnail, Data->Description); // �ӽ�

		if (Data->ItemType == EItemType::Equipment)
		{
			ShopWidget->AddItem(EntryData, EShopCategoryTab::Equipment);
		}

		ShopEquipmentItemIdList.TryAdd(Id);
	}
}

void AShop::OnShopItemListChanged(const FShopItemListChangeInfo& Info)
{
	LOGVN(Error, TEXT("Changed!"));
	
	FFADItemDataRow*& Data = DataTableArray[Info.ItemIdAfter];

	UShopItemEntryData* EntryData = NewObject<UShopItemEntryData>();
	EntryData->Init(Data->Price, Data->Thumbnail, Data->Description); // �ӽ�

	if (Data->ItemType == EItemType::Consumable)
	{
		ShopWidget->AddItem(EntryData, EShopCategoryTab::Consumable);
	}
	else if (Data->ItemType == EItemType::Equipment)
	{
		ShopWidget->AddItem(EntryData, EShopCategoryTab::Equipment);
	}

	ShopWidget->RefreshItemView();
}

bool AShop::HasItem(int32 ItemId)
{
	return (ShopConsumableItemIdList.Contains(ItemId) != INDEX_NONE);
}

bool AShop::IsItemMeshCached(int32 ItemId)
{
	return CachedMeshList.Contains(ItemId);
}