#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "ShopItemEntryData.generated.h"

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UShopItemEntryData : public UObject
{
	GENERATED_BODY()

#pragma region Methods



public:

	void Init(int32 NewPrice, UTexture2D* NewItemImage, const FString& NewToolTipText);

#pragma endregion

#pragma region Variables

private:

	UPROPERTY()
	TObjectPtr<UTexture2D> ItemImage;

	int32 Price = 0;
	FString ToolTipText;

#pragma endregion

#pragma region Getters, Setters

public:

	int32 GetPrice() const;
	UTexture2D* GetItemImage() const;
	const FString& GetToolTipText() const;
#pragma endregion

};
