#include "Shops/Shop.h"

#include "Character/UnitBase.h"
#include "Shops/ShopWidgets/ShopCategoryTabWidget.h"
#include "AbyssDiverUnderWorld.h"

AShop::AShop()
{
	PrimaryActorTick.bCanEverTick = false;

}

void AShop::BeginPlay()
{
	Super::BeginPlay();
}

void AShop::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

EBuyResult AShop::BuyItem(int32 ItemId)
{
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

ESellResult AShop::SellItem(int32 ItemId, class AUnitBase* Seller)
{
	/*if (������ �� ���� ������)
	{
		return ESellResult::NotExistItem;
	}*/

	// �� �߰�
	// �κ��丮���� ������ ����

	return ESellResult::Succeeded;
}

void AShop::InitData()
{
	// ������ ������ ���̺�κ��� ������ Id ����Ʈ�� ���� ������ �����;� ��.
}

void AShop::OnCategoryTabClicked(ECategoryTab CategoryTab)
{
	if (CategoryTab == ECategoryTab::None)
	{
		LOGV(Error, TEXT("ShopCategoryTab is not Defined Category"));
		return;
	}


}

bool AShop::HasItem(int32 ItemId)
{
	return ShopItemIdList.Contains(ItemId);
}

bool AShop::IsItemMeshCached(int32 ItemId)
{
	return CachedMeshList.Contains(ItemId);
}

