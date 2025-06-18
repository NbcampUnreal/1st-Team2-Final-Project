#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "ShopItemEntryData.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEntryUpdatedFromDataDelegate, class UShopItemSlotWidget* /*ShopItemSlotWidget*/);
/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UShopItemEntryData : public UObject
{
	GENERATED_BODY()

#pragma region Methods, Delegates

public:

	void Init(int32 NewSlotIndex, UTexture2D* NewItemImage, const FString& NewToolTipText);

	FOnEntryUpdatedFromDataDelegate OnEntryUpdatedFromDataDelegate;

#pragma endregion

#pragma region Variables

protected:

	UPROPERTY()
	TObjectPtr<UTexture2D> ItemImage;

	FString ToolTipText;
	int32 SlotIndex;

#pragma endregion

#pragma region Getters, Setters

public:

	UTexture2D* GetItemImage() const;
	const FString& GetToolTipText() const;

	int32 GetSlotIndex() const;
	void SetSlotIndex(int32 NewSlotIndex);

#pragma endregion

};
