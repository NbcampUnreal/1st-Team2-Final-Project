#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"

#include "ShopItemSlotWidget.generated.h"



class UImage;
/**
 * �������� �Һ���, ���, ���� ǥ���ϴ� ����
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


private:

	void ShowOrNotToolTip(bool bShouldShow);

#pragma endregion

#pragma region Variables

private:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> SlotImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ToolTipImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class URichTextBlock> ToolTipTextBlock;

	FTimerHandle ToolTipTimerHandle;

#pragma endregion

#pragma region Getters, Setters

public:

	void SetSlotImage(UTexture2D* NewTexture);
	void SetToolTipText(const FString& NewText);


#pragma endregion


};
