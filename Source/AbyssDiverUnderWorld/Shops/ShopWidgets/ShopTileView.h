#pragma once

#include "CoreMinimal.h"
#include "Components/TileView.h"

#include "ShopTileView.generated.h"

class UShopItemEntryData;
/**
 *  상점 아이템 목록 띄우는 위젯
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UShopTileView : public UTileView
{
	GENERATED_BODY()


public:

	void SetAllElements(const TArray<UShopItemEntryData*>& Elements);
	
};
