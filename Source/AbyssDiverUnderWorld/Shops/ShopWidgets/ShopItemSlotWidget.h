#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"

#include "ShopItemSlotWidget.generated.h"

DECLARE_DELEGATE_OneParam(FOnShopItemSlotWidgetClickedDelegate, int32 /*SlotIndex*/);

class URichTextBlock;
class UImage;
/**
 * 상점에서 소비템, 장비, 업글 표시하는 슬롯
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UShopItemSlotWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	
protected:

	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

#pragma region Methods, Delegates

public:

	FOnShopItemSlotWidgetClickedDelegate OnShopItemSlotWidgetClickedDelegate;

private:

	void UpdateToolTipVisibility(bool bShouldShow);

#pragma endregion

#pragma region Variables

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> SlotImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ToolTipImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class URichTextBlock> ToolTipTextBlock;

	// 시간 측정 필요할까
	FTimerHandle ToolTipTimerHandle;

	int32 SlotIndex;

#pragma endregion

#pragma region Getters, Setters

public:

	void SetSlotImage(UTexture2D* NewTexture);
	void SetToolTipText(const FString& NewText);
	void SetSlotIndex(int32 NewSlotIndex);

#pragma endregion


};
