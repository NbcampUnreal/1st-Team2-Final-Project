#include "ShopItemSlotWidget.h"

FReply UShopItemSlotWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

    // �Ƹ� Ŭ�� �ϸ� ����?

    return FReply::Handled();
}

void UShopItemSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
    // TODO : �ð� �����ϰ� ���� ����
}

void UShopItemSlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseLeave(InMouseEvent);

    // TODO : �ð� ���� �ʱ�ȭ
}
