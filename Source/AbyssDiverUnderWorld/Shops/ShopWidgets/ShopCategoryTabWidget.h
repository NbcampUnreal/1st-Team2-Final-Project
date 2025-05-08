#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "ShopCategoryTabWidget.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnShopCategoryTabClickedDelegate, ECategoryTab);

UENUM()
enum class ECategoryTab : uint8
{
	None,
	Consumable,
	Weapons,
	Upgrade,
	MAX
};

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UShopCategoryTabWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	virtual void NativeConstruct() override;

#pragma region Methods And Delegate
public:

	FOnShopCategoryTabClickedDelegate OnShopCategoryTabClickedDelegate;

private:

	UFUNCTION()
	void OnCategoryButtonClicked();


#pragma endregion

#pragma region Variables

private:

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget), Category = "ShopCategoryTabWidget")
	TObjectPtr<class UButton> TabButton;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess), Category = "ShopCategoryTabWidget")
	ECategoryTab CurrentCategoryTab = ECategoryTab::None;

#pragma endregion
};
