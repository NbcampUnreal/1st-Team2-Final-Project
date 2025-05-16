#include "ShopCategoryTabWidget.h"

#include "AbyssDiverUnderWorld.h"

#include "Components/Button.h"

void UShopCategoryTabWidget::NativeConstruct()
{
	if (TabButton->OnClicked.IsBound() == false)
	{
		TabButton->OnClicked.AddDynamic(this, &UShopCategoryTabWidget::OnCategoryButtonClicked);
	}
}

void UShopCategoryTabWidget::OnCategoryButtonClicked()
{
	OnShopCategoryTabClickedDelegate.Broadcast(CurrentCategoryTab);
}
