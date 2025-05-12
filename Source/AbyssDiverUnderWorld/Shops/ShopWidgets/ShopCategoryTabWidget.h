#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "ShopCategoryTabWidget.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnShopCategoryTabClickedDelegate, EShopCategoryTab);

UENUM()
enum class EShopCategoryTab : uint8
{
	Consumable = 0,
	Equipment,
	Upgrade,
	Max
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

#pragma region Methods And Delegates
public:

	FOnShopCategoryTabClickedDelegate OnShopCategoryTabClickedDelegate;

private:

	UFUNCTION()
	void OnCategoryButtonClicked();


#pragma endregion

#pragma region Variables

private:

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess), Category = "ShopCategoryTabWidget")
	EShopCategoryTab CurrentCategoryTab = EShopCategoryTab::Max;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget), Category = "ShopCategoryTabWidget")
	TObjectPtr<class UButton> TabButton;

#pragma endregion
};
