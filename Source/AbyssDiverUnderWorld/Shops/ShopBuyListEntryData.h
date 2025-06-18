#pragma once

#include "CoreMinimal.h"
#include "Shops/ShopItemEntryData.h"

#include "ShopBuyListEntryData.generated.h"

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UShopBuyListEntryData : public UShopItemEntryData
{
	GENERATED_BODY()

#pragma region Variables

protected:

	int32 ItemCount;

#pragma endregion

#pragma region Getters / Setters

public:

	int32 GetItemCount();
	void SetItemCount(int32 NewItemCount);

#pragma endregion
};
