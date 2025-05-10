#include "ShopWidget.h"

#include "ShopCategoryTabWidget.h"
#include "AbyssDiverUnderWorld.h"
#include "ShopTileView.h"
#include "Shops/ShopItemEntryData.h"

void UShopWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CurrentActivatedTab = EShopCategoryTab::Consumable;

	ConsumableTab->OnShopCategoryTabClickedDelegate.AddUObject(this, &UShopWidget::OnCategoryTabClicked);
	EquipmentTab->OnShopCategoryTabClickedDelegate.AddUObject(this, &UShopWidget::OnCategoryTabClicked);
}

void UShopWidget::SetAllItems(const TArray<UShopItemEntryData*>& EntryDataList, EShopCategoryTab Tab)
{
	if (Tab >= EShopCategoryTab::Max)
	{
		LOGV(Error, TEXT("Weird Tab Type : %d"), Tab);
		return;
	}

	switch (Tab)
	{
	case EShopCategoryTab::Consumable:
		ConsumableTabEntryDataList = EntryDataList;
		break;
	case EShopCategoryTab::Equipment:
		EquipmentTabEntryDataList = EntryDataList;
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
}

void UShopWidget::AddItem(UShopItemEntryData* EntryData, EShopCategoryTab Tab)
{
	if (Tab >= EShopCategoryTab::Max)
	{
		LOGV(Error, TEXT("Weird Tab Type : %d"), Tab);
		return;
	}

	switch (Tab)
	{
	case EShopCategoryTab::Consumable:
		ConsumableTabEntryDataList.Add(EntryData);
		break;
	case EShopCategoryTab::Equipment:
		EquipmentTabEntryDataList.Add(EntryData);
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
}

void UShopWidget::ShowItemViewForTab(EShopCategoryTab TabType)
{
	if (TabType >= EShopCategoryTab::Max)
	{
		LOGV(Error, TEXT("Weird Tab Type : %d"), TabType);
		return;
	}

	if (TabType != CurrentActivatedTab)
	{
		SetCurrentActivatedTab(TabType);
	}

	RefreshItemView();
}

void UShopWidget::RefreshItemView()
{
	check(ItemTileView);

	switch (CurrentActivatedTab)
	{
	case EShopCategoryTab::Consumable:
		ItemTileView->SetAllElements(ConsumableTabEntryDataList);
		break;
	case EShopCategoryTab::Equipment:
		ItemTileView->SetAllElements(EquipmentTabEntryDataList);
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
}

void UShopWidget::OnCategoryTabClicked(EShopCategoryTab CategoryTab)
{
	if (CategoryTab >= EShopCategoryTab::Max)
	{
		LOGV(Error, TEXT("Weird Tab Type : %d"), CategoryTab);
		return;
	}

	ShowItemViewForTab(CategoryTab);
}

EShopCategoryTab UShopWidget::GetCurrentActivatedTab() const
{
	return CurrentActivatedTab;
}

void UShopWidget::SetCurrentActivatedTab(EShopCategoryTab Tab)
{
	if (Tab >= EShopCategoryTab::Max)
	{
		LOGV(Error, TEXT("Weird Tab Type : %d"), Tab);
		return;
	}

	CurrentActivatedTab = Tab;
}

UShopCategoryTabWidget* UShopWidget::GetCategoryTab(EShopCategoryTab CategoryTab) const
{
	if (CategoryTab >= EShopCategoryTab::Max)
	{
		LOGV(Error, TEXT("Weird Tab Type : %d"), CategoryTab);
		return nullptr;
	}

	UShopCategoryTabWidget* Tab = nullptr;

	switch (CategoryTab)
	{
	case EShopCategoryTab::Consumable:
		Tab = ConsumableTab;
		break;
	case EShopCategoryTab::Equipment:
		Tab = EquipmentTab;
		break;
	case EShopCategoryTab::Upgrade:
		LOGV(Error, TEXT("Upgrade Tab is not Supported Currently"));
		return nullptr;
	case EShopCategoryTab::Max:
		check(false);
		return nullptr;
	default:
		check(false);
		return nullptr;
	}

	return Tab;
}