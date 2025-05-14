#include "ShopWidget.h"

#include "ShopCategoryTabWidget.h"
#include "AbyssDiverUnderWorld.h"
#include "ShopTileView.h"
#include "Shops/ShopItemEntryData.h"
#include "Shops/ShopWidgets/ShopElementInfoWidget.h"
#include "Components/Button.h"

void UShopWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CurrentActivatedTab = EShopCategoryTab::Consumable;

	ConsumableTab->OnShopCategoryTabClickedDelegate.AddUObject(this, &UShopWidget::OnCategoryTabClicked);
	EquipmentTab->OnShopCategoryTabClickedDelegate.AddUObject(this, &UShopWidget::OnCategoryTabClicked);
	CloseButton->OnClicked.AddDynamic(this, &UShopWidget::OnCloseButtonClicked);
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

void UShopWidget::RemoveItem(int32 Index, EShopCategoryTab Tab)
{
	if (Tab >= EShopCategoryTab::Max)
	{
		LOGV(Error, TEXT("Weird Tab Type : %d"), Tab);
		return;
	}

	switch (Tab)
	{
	case EShopCategoryTab::Consumable:
		ConsumableTabEntryDataList.RemoveAt(Index);
		break;
	case EShopCategoryTab::Equipment:
		EquipmentTabEntryDataList.RemoveAt(Index);
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

void UShopWidget::ModifyItem(int32 Index, UTexture2D* NewItemImage, const FString& NewToolTipText, EShopCategoryTab Tab)
{
	if (Tab >= EShopCategoryTab::Max)
	{
		LOGV(Error, TEXT("Weird Tab Type : %d"), Tab);
		return;
	}

	switch (Tab)
	{
	case EShopCategoryTab::Consumable:
		ConsumableTabEntryDataList[Index]->Init(Index, NewItemImage, NewToolTipText);
		break;
	case EShopCategoryTab::Equipment:
		EquipmentTabEntryDataList[Index]->Init(Index, NewItemImage, NewToolTipText);
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

void UShopWidget::ShowItemInfos(USkeletalMesh* NewItemMesh, const FString& NewDescription, const FString& NewInfoText)
{
	InfoWidget->ShowItemInfos(NewItemMesh, NewDescription, NewInfoText);
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

void UShopWidget::OnCloseButtonClicked()
{
	OnShopCloseButtonClickedDelegate.Broadcast();
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

UShopElementInfoWidget* UShopWidget::GetInfoWidget() const
{
	check(InfoWidget);
	return InfoWidget;
}
