#pragma once

#include "CoreMinimal.h"
#include "Shops/ShopWidgets/ShopItemSlotWidget.h"

#include "ShopBuyListSlotWidget.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UShopBuyListSlotWidget : public UShopItemSlotWidget
{
	GENERATED_BODY()

protected:

	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

#pragma region Variables

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemCountText;

	int32 ItemCount;

#pragma endregion

#pragma region Getters / Setters

public:

	void SetItemCountText(int32 NewItemCount);

#pragma endregion

};
