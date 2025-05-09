#include "ShopCategoryTabWidget.h"

#include "Components/Button.h"

void UShopCategoryTabWidget::NativeConstruct()
{
	TabButton->OnClicked.AddDynamic(this, &UShopCategoryTabWidget::OnCategoryButtonClicked);
}

void UShopCategoryTabWidget::OnCategoryButtonClicked()
{
	OnShopCategoryTabClickedDelegate.Broadcast(CurrentCategoryTab);
}
