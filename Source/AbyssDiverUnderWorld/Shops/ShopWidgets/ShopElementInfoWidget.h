#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "ShopElementInfoWidget.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnBuyButtonClickedDelegate);
class URichTextBlock;

/**
 * 상점에서 아이템, 장비, 업글에 대한 정보를 띄우는 위젯
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UShopElementInfoWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	virtual void NativeConstruct() override;
	
#pragma region Methods, Delegates

public:

	void Init(UStaticMeshComponent* NewItemMeshComp);

	void ShowItemInfos(UStaticMesh* NewItemMesh, const FString& NewDescription, const FString& NewInfoText);

	void ChangeItemDescription(const FString& NewDescription);
	void ChangeInfoText(const FString& NewInfoText);
	void ChangeItemMesh(UStaticMesh* NewMesh);

	void SetDescriptionActive(bool bShouldActivate);
	void SetInfoTextActive(bool bShouldActivate);
	void SetItemMeshActive(bool bShouldActivate);

	FOnBuyButtonClickedDelegate OnBuyButtonClickedDelegate;

private:

	UFUNCTION()
	void OnBuyButtonClicked();
#pragma endregion

#pragma region Variables

protected:

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> ItemMeshComponent;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> ItemMeshImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> DescriptionText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> InfoText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> BuyButton;

#pragma endregion
};
