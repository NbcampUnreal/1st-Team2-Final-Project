#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "ShopElementInfoWidget.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnBuyButtonClickedDelegate);

class URichTextBlock;
class UShopItemMeshPanel;
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

	void Init(USkeletalMeshComponent* NewItemMeshComp);

	void ShowItemInfos(USkeletalMesh* NewItemMesh, const FString& NewDescription, const FString& NewInfoText);
	void ShowUpgradeInfos(USkeletalMesh* NewUpgradeItemMesh, int32 CurrentUpgradeLevel, bool bIsMaxLevel, int32 CurrentUpgradeCost, const FString& ExtraInfoText);

	void ChangeItemDescription(const FString& NewDescription);
	void ChangeInfoText(const FString& NewInfoText);
	void ChangeItemMesh(USkeletalMesh* NewMesh);

	void ChangeUpgradeLevelInfo(int32 CurrentLevel, bool bIsMaxLevel);
	void ChangeUpgradeCostInfo(int32 CurrentUpgradeCost);

	void SetDescriptionActive(bool bShouldActivate);
	void SetInfoTextActive(bool bShouldActivate);
	void SetItemMeshActive(bool bShouldActivate);
	void SetBuyButtonActive(bool bShouldActivate);

	void SetUpgradeLevelInfoActive(bool bShouldActivate);
	void SetUpgradeCostInfo(bool bShouldActivate);

	FOnBuyButtonClickedDelegate OnBuyButtonClickedDelegate;

private:

	UFUNCTION()
	void OnBuyButtonClicked();

#pragma endregion

#pragma region Variables

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> DescriptionText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> InfoText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> UpgradeLevelInfoText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> UpgradeCostInfoText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> BuyButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UShopItemMeshPanel> ItemMeshPanel;

#pragma endregion

#pragma region Getter / Setter

public:
	UShopItemMeshPanel* GetItemMeshPanel() const;

#pragma endregion

};
