#include "ShopWidget.h"

#include "ShopCategoryTabWidget.h"
#include "AbyssDiverUnderWorld.h"

UShopCategoryTabWidget* UShopWidget::GetCategoryTab(ECategoryTab CategoryTab) const
{
	if (CategoryTab == ECategoryTab::None || CategoryTab == ECategoryTab::MAX)
	{
		LOGV(Error, TEXT("Weird Category : %d"), CategoryTab);
		return nullptr;
	}

	int32 Index = (int32)CategoryTab - 1;
	if (CategoryTabList.IsValidIndex(Index) == false)
	{
		LOGV(Error, TEXT("Not Valid Index : %d"), Index);
		return nullptr;
	}

	return CategoryTabList[(int32)CategoryTab - 1];
}
