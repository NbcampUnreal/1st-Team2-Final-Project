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

	virtual void NativeConstruct() override;

#pragma region Methods

public:

	void SetAllItems(const TArray<UShopItemEntryData*>& EntryDataList, EShopCategoryTab Tab);
	void AddItem(UShopItemEntryData* EntryData, EShopCategoryTab Tab);
	void RemoveItem(int32 Index, EShopCategoryTab Tab);
	void ModifyItem(int32 Index, UTexture2D* NewItemImage, const FString& NewToolTipText, EShopCategoryTab Tab);

	void ShowItemViewForTab(EShopCategoryTab TabType);
	void RefreshItemView();

	void ShowItemInfos(USkeletalMesh* NewItemMesh, const FString& NewDescription, const FString& NewInfoText);
	void ShowUpgradeInfos(USkeletalMesh* NewUpgradeItemMesh, int32 CurrentUpgradeLevel, bool bIsMaxLevel, int32 CurrentUpgradeCost, const FString& ExtraInfoText);

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

	UPROPERTY()
	TArray<TObjectPtr<UShopItemEntryData>> ConsumableTabEntryDataList;

	UPROPERTY()
	TArray<TObjectPtr<UShopItemEntryData>> EquipmentTabEntryDataList;

	UPROPERTY()
	TArray<TObjectPtr<UShopItemEntryData>> UpgradeTabEntryDataList;

	EShopCategoryTab CurrentActivatedTab;

	const int8 MAX_TAB_COUNT = 3;

#pragma endregion

#pragma region Getters, Setters

public:

	EShopCategoryTab GetCurrentActivatedTab() const;
	void SetCurrentActivatedTab(EShopCategoryTab Tab);

	UShopCategoryTabWidget* GetCategoryTab(EShopCategoryTab CategoryTab) const;

	UShopElementInfoWidget* GetInfoWidget() const;

	TArray<TObjectPtr<UShopItemEntryData>>& GetUpgradeTabEntryDataList();
	
#pragma endregion
	
};
