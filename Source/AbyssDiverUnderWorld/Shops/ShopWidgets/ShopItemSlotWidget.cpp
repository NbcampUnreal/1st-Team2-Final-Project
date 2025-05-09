#include "ShopItemSlotWidget.h"

FReply UShopItemSlotWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

    // 아마 클릭 하면 구매?

    return FReply::Handled();
}

void UShopItemSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
    // TODO : 시간 측정하고 툴팁 띄우기
}

void UShopItemSlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseLeave(InMouseEvent);

    // TODO : 시간 측정 초기화
}
