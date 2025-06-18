#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "ShopWidget.generated.h"

enum class EShopCategoryTab : uint8;
enum class EUpgradeType : uint8;
class UShopCategoryTabWidget;
class UShopItemEntryData;
class UShopElementInfoWidget;
class UShopTileView;
class UShopBuyListEntryData;
class UButton;
class UTextBlock;
class UWidgetAnimation;

DECLARE_MULTICAST_DELEGATE(FOnShopCloseButtonClickedDelegate);
DECLARE_MULTICAST_DELEGATE(FOnShopBuyButtonClickedDelegate);

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UShopWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

#pragma region Methods

public:

	void SetAllItems(const TArray<UShopItemEntryData*>& EntryDataList, EShopCategoryTab Tab);
	void AddItem(UShopItemEntryData* EntryData, EShopCategoryTab Tab);
	void RemoveItem(int32 Index, EShopCategoryTab Tab);
	void ModifyItem(int32 Index, UTexture2D* NewItemImage, const FString& NewToolTipText, EShopCategoryTab Tab);

	void ShowItemViewForTab(EShopCategoryTab TabType);
	void RefreshItemView();

	void ShowItemInfos(int32 ItemId);
	void ShowUpgradeInfos(EUpgradeType UpgradeType, uint8 Grade, bool bIsMaxLevel);

	void SetTeamMoneyText(int32 NewTeamMoney);

	void PlayCloseAnimation();

	// return : 추가된 Entry data 반환, InOutIncreasedAmount : 증가할 양, 이후 실질적으로 증가한 량 반환
	UShopBuyListEntryData* AddToBuyList(uint8 ItemId, int32& InOutIncreasedAmount);
	void RemoveBuyListAt(int32 ListIndex, int32 Amount);
	void RemoveBuyListAll();

	// 존재하지 않으면 INDEX_NONE 반환, 존재하면 ListIndex 반환
	int32 Contains(uint8 ItemId);

	void ChangeTotalPriceText(int32 NewTotalPrice);

	FOnShopCloseButtonClickedDelegate OnShopCloseButtonClickedDelegate;
	FOnShopBuyButtonClickedDelegate OnShopBuyButtonClickedDelegate;

private:

	UFUNCTION()
	void OnCategoryTabClicked(EShopCategoryTab CategoryTab);

	UFUNCTION()
	void OnCloseButtonClicked();

	UFUNCTION()
	void OnBuyButtonClicked();
	
#pragma endregion

#pragma region Variables

private:

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget), meta = (AllowPrivateAccess), Category = "ShopWidget")
	TObjectPtr<UShopCategoryTabWidget> ConsumableTab;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget), meta = (AllowPrivateAccess), Category = "ShopWidget")
	TObjectPtr<UShopCategoryTabWidget> EquipmentTab;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget), meta = (AllowPrivateAccess), Category = "ShopWidget")
	TObjectPtr<UShopCategoryTabWidget> UpgradeTab;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UShopTileView> ItemTileView;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UShopTileView> UpgradeTileView;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UShopTileView> BuyListTileView;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UShopElementInfoWidget> InfoWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BuyButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TeamMoneyText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TotalPriceText;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> CloseShopAnim;

	UPROPERTY()
	TArray<TObjectPtr<UShopItemEntryData>> ConsumableTabEntryDataList;

	UPROPERTY()
	TArray<TObjectPtr<UShopItemEntryData>> EquipmentTabEntryDataList;

	UPROPERTY()
	TArray<TObjectPtr<UShopItemEntryData>> UpgradeTabEntryDataList;

	UPROPERTY()
	TArray<TObjectPtr<UShopBuyListEntryData>> BuyListEntryDataList;
	TArray<uint8> BuyListEntryItemIdList;

	EShopCategoryTab CurrentActivatedTab;

	const int8 MAX_TAB_COUNT = 3;
	const float MESH_ROTATION_SPEED = 0.5f;
	static const int32 MaxItemCount;
#pragma endregion

#pragma region Getters, Setters

public:

	EShopCategoryTab GetCurrentActivatedTab() const;
	void SetCurrentActivatedTab(EShopCategoryTab Tab);

	UShopCategoryTabWidget* GetCategoryTab(EShopCategoryTab CategoryTab) const;

	UShopElementInfoWidget* GetInfoWidget() const;

	TArray<TObjectPtr<UShopItemEntryData>>& GetUpgradeTabEntryDataList();
	
	float GetCloseShopAnimEndTime() const;
#pragma endregion
	
};
