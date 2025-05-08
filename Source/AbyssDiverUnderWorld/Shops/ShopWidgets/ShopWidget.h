#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "ShopWidget.generated.h"


enum class ECategoryTab : uint8;
class UShopCategoryTabWidget;
/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UShopWidget : public UUserWidget
{
	GENERATED_BODY()


private:

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess), Category = "ShopWidget")
	TArray<TObjectPtr<UShopCategoryTabWidget>> CategoryTabList;

public:

	UShopCategoryTabWidget* GetCategoryTab(ECategoryTab CategoryTab) const;
	
};
