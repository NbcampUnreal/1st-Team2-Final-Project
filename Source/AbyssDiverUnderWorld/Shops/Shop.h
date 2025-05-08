#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Shop.generated.h"

USTRUCT()
struct FShopItemData
{
	GENERATED_BODY()

	UPROPERTY();
	TObjectPtr<UTexture2D> ItemImageTexture;

	int32 ItemId;
	
	int32 ItemPrice;

	int32 StatValue; // 공격력, 힐량 등

	FString Description;
};

enum class EShopTapType
{
	Consumable,
	Weapons,
	Upgrade
};

enum class EBuyResult
{
	Succeeded,
	NotEnoughMoney,
	NotExistItem,
};

enum class ESellResult
{
	Succeeded,
	NotExistItem
};

enum class ECategoryTab : uint8;
class UShopWidget;

UCLASS()
class ABYSSDIVERUNDERWORLD_API AShop : public AActor
{
	GENERATED_BODY()
	
public:	

	AShop();

protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

#pragma region Methods

public:

	EBuyResult BuyItem(int32 ItemId);
	ESellResult SellItem(int32 ItemId, class AUnitBase* Seller);

protected:

	void InitData();

private:

	UFUNCTION()
	void OnCategoryTabClicked(ECategoryTab CategoryTab);

	bool HasItem(int32 ItemId);
	bool IsItemMeshCached(int32 ItemId);


#pragma endregion

#pragma region Variables

protected:

	UPROPERTY(EditAnywhere, Category = "Shop")
	TArray<int32> ShopItemIdList;

	UPROPERTY()
	TObjectPtr<UShopWidget> ShopWidget;

	TArray<FShopItemData> ShopItemDataList;
	
private:

	// ItemId, Mesh류(SM, SKM)
	UPROPERTY()
	TMap<int32, TObjectPtr<UObject>> CachedMeshList;

#pragma endregion


};
