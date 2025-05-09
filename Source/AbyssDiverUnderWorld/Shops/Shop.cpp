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

ESellResult AShop::SellItem(int32 ItemId, class AUnitBase* Seller)
{
	/*if (아이템 안 갖고 있으면)
	{
		return ESellResult::NotExistItem;
	}*/

	// 돈 추가
	// 인벤토리에서 아이템 삭제

	return ESellResult::Succeeded;
}

void AShop::InitData()
{
	// 아이템 데이터 테이블로부터 아이템 Id 리스트를 통해 데이터 가져와야 함.
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

