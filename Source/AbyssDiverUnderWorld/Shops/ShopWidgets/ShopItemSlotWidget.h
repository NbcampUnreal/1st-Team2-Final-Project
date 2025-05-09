#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "ShopItemSlotWidget.generated.h"

/**
 * 상점에서 소비템, 장비, 업글 표시하는 슬롯
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
