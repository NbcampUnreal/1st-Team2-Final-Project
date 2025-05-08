#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "ShopItemSlotWidget.generated.h"

/**
 * �������� �Һ���, ���, ���� ǥ���ϴ� ����
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UShopItemSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
};
