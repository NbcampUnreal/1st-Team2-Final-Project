#pragma once

#include "CoreMinimal.h"
#include "Components/TileView.h"

#include "ShopTileView.generated.h"

class UShopItemEntryData;
/**
 *  ���� ������ ��� ���� ����
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UShopTileView : public UTileView
{
	GENERATED_BODY()


public:

	void SetAllElements(const TArray<UShopItemEntryData*>& Elements);
	
};
