#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "ShopWidget.generated.h"


enum class EShopCategoryTab : uint8;
class UShopCategoryTabWidget;
class UShopItemEntryData;
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

	void ShowItemViewForTab(EShopCategoryTab TabType);
	void RefreshItemView();

private:

	UFUNCTION()
	void OnCategoryTabClicked(EShopCategoryTab CategoryTab);
	
#pragma endregion

#pragma region Variables

private:


	UPROPERTY(EditDefaultsOnly, meta = (BindWidget), meta = (AllowPrivateAccess), Category = "ShopWidget")
	TObjectPtr<UShopCategoryTabWidget> ConsumableTab;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget), meta = (AllowPrivateAccess), Category = "ShopWidget")
	TObjectPtr<UShopCategoryTabWidget> EquipmentTab;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UShopTileView> ItemTileView;

	EShopCategoryTab CurrentActivatedTab;

	UPROPERTY()
	TArray<TObjectPtr<UShopItemEntryData>> ConsumableTabEntryDataList;

	UPROPERTY()
	TArray<TObjectPtr<UShopItemEntryData>> EquipmentTabEntryDataList;

	const int8 MAX_TAB_COUNT = 3;

#pragma endregion

#pragma region Getters, Setters

public:

	EShopCategoryTab GetCurrentActivatedTab() const;
	void SetCurrentActivatedTab(EShopCategoryTab Tab);


	UShopCategoryTabWidget* GetCategoryTab(EShopCategoryTab CategoryTab) const;
	
#pragma endregion
	
};
