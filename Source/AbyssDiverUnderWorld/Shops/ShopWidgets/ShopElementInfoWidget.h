#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "ShopElementInfoWidget.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnBuyButtonClickedDelegate, int32/*Buy Quantity*/);

class URichTextBlock;
class UShopItemMeshPanel;
class UButton;

enum class EUpgradeType : uint8;

/**
 * 상점에서 아이템, 장비, 업글에 대한 정보를 띄우는 위젯
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UShopElementInfoWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
#pragma region Methods, Delegates

public:

	void Init(USkeletalMeshComponent* NewItemMeshComp);

	void ShowItemInfos(int32 ItemId);
	void ShowUpgradeInfos(EUpgradeType UpgradeType, uint8 Grade, bool bIsMaxLevel);
	void ChangeItemDescription(const FString& NewDescription);
	void ChangeNameInfoText(const FString& NewInfoText);
	void ChangeItemMesh(USkeletalMesh* NewMesh, int32 ItemId);

	void ChangeUpgradeLevelInfo(int32 CurrentLevel, bool bIsMaxLevel);
	void ChangeCostInfo(int32 Cost, bool bIsUpgradeCost);

	void ChangeCurrentQuantityNumber(int32 NewNumber);

	void ChangeRemainingMoneyAfterPurchaseTextFromCost(int32 Cost);
	void ChangeRemainingMoneyAfterPurchaseText(int32 MoneyAmount);

	void SetDescriptionActive(bool bShouldActivate);
	void SetNameInfoTextActive(bool bShouldActivate);
	void SetItemMeshActive(bool bShouldActivate);
	void SetBuyButtonActive(bool bShouldActivate);

	void SetUpgradeLevelInfoActive(bool bShouldActivate);
	void SetCostInfoActive(bool bShouldActivate);

	void SetQuantityOverlayActive(bool bShouldActivate);

	void SetRemainingMoneyAfterPurchaseTextActive(bool bShouldActivate);

	FOnBuyButtonClickedDelegate OnBuyButtonClickedDelegate;

private:

	UFUNCTION()
	void OnBuyButtonClicked();

	UFUNCTION()
	void OnIncreaseButtonClicked();

	UFUNCTION()
	void OnDecreaseButtonClicked();

	void OnTeamCreditChanged(int32 ChangedValue);

#pragma endregion

#pragma region Variables

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> NameInfoText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> DescriptionText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> UpgradeLevelInfoText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> CostInfoText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BuyButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UShopItemMeshPanel> ItemMeshPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> IncreaseButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> DecreaseButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> CurrentQuantityNumberText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UOverlay> QuantityOverlay;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URichTextBlock> RemainingMoneyAfterPurchaseText;

private:

	int32 CurrentQuantityNumber = 0;
	int32 CurrentCost = INT_MAX;

	uint8 bIsStackableItem : 1;
	uint8 bIsShowingUpgradeView : 1;

	const int32 MAX_ITEM_COUNT = 99;

#pragma endregion

#pragma region Getter / Setter

public:

	UShopItemMeshPanel* GetItemMeshPanel() const;

#pragma endregion

};
