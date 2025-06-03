#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "ShopWidget.generated.h"

enum class EShopCategoryTab : uint8;
class UShopCategoryTabWidget;
class UShopItemEntryData;
class UShopElementInfoWidget;
class UShopTileView;

DECLARE_MULTICAST_DELEGATE(FOnShopCloseButtonClickedDelegate);

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
	void ShowUpgradeInfos(USkeletalMesh* NewUpgradeItemMesh, int32 CurrentUpgradeLevel, bool bIsMaxLevel, int32 CurrentUpgradeCost, const FString& ExtraInfoText);

	void SetTeamMoneyText(int32 NewTeamMoney);

	void PlayCloseAnimation();

	FOnShopCloseButtonClickedDelegate OnShopCloseButtonClickedDelegate;

private:

	UFUNCTION()
	void OnCategoryTabClicked(EShopCategoryTab CategoryTab);

	UFUNCTION()
	void OnCloseButtonClicked();
	
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
	TObjectPtr<UShopElementInfoWidget> InfoWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> CloseButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class URichTextBlock> TeamMoneyText;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<class UWidgetAnimation> CloseShopAnim;

	UPROPERTY()
	TArray<TObjectPtr<UShopItemEntryData>> ConsumableTabEntryDataList;

	UPROPERTY()
	TArray<TObjectPtr<UShopItemEntryData>> EquipmentTabEntryDataList;

	UPROPERTY()
	TArray<TObjectPtr<UShopItemEntryData>> UpgradeTabEntryDataList;

	EShopCategoryTab CurrentActivatedTab;

	const int8 MAX_TAB_COUNT = 3;
	const float MESH_ROTATION_SPEED = 0.5f;

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
