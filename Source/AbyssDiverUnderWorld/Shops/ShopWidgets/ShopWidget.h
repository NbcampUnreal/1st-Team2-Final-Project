#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "ShopWidget.generated.h"

enum class EShopCategoryTab : uint8;
class UShopCategoryTabWidget;
class UShopItemEntryData;
class UShopElementInfoWidget;

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

	void ShowItemInfos(UStaticMesh* NewItemMesh, const FString& NewDescription, const FString& NewInfoText);

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

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UShopTileView> ItemTileView;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UShopElementInfoWidget> InfoWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> CloseButton;

	UPROPERTY()
	TArray<TObjectPtr<UShopItemEntryData>> ConsumableTabEntryDataList;

	UPROPERTY()
	TArray<TObjectPtr<UShopItemEntryData>> EquipmentTabEntryDataList;

	EShopCategoryTab CurrentActivatedTab;

	const int8 MAX_TAB_COUNT = 3;

#pragma endregion

#pragma region Getters, Setters

public:

	EShopCategoryTab GetCurrentActivatedTab() const;
	void SetCurrentActivatedTab(EShopCategoryTab Tab);


	UShopCategoryTabWidget* GetCategoryTab(EShopCategoryTab CategoryTab) const;

	UShopElementInfoWidget* GetInfoWidget() const;
	
#pragma endregion
	
};
