#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Net/Serialization/FastArraySerializer.h"

#include "Shop.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnShopItemListChangedDelegate, const FShopItemListChangeInfo&);

USTRUCT()
struct FShopItemData
{
	GENERATED_BODY()

	UPROPERTY();
	TObjectPtr<UTexture2D> ItemImageTexture;

	uint8 ItemId;
	
	int32 ItemPrice;

	int32 StatValue; // 공격력, 힐량 등

	FString Description;
};
#pragma region Enums

enum class EBuyResult
{
	Succeeded,
	NotEnoughMoney,
	NotExistItem,
	HasNoAuthority,
	FailedFromOtherReason,
	Max
};

enum class ESellResult
{
	Succeeded,
	NotExistItem,
	HasNoAuthority,
	FailedFromOtherReason,
	Max
};

enum class EShopItemChangeType
{
	Added,
	Removed,
	Modified,
	Max
};

#pragma endregion

class AShop;
class UShopWidget;
class UShopItemEntryData;

struct FFADItemDataRow;
struct FShopItemIdList;

enum class EShopCategoryTab : uint8;

#pragma region FastArraySerializer

USTRUCT()
struct FShopItemListChangeInfo
{
	GENERATED_BODY()

	FShopItemListChangeInfo()
	{
	}

	FShopItemListChangeInfo(EShopCategoryTab InTab, int16 InShopIndex, uint8 InItemId, EShopItemChangeType InChangeType)
	{
		WhichTab = InTab;
		ShopIndex = InShopIndex;
		ItemIdAfter = InItemId;
		ChangeType = InChangeType;
	}

	EShopCategoryTab WhichTab;
	int16 ShopIndex;
	uint8 ItemIdAfter;
	EShopItemChangeType ChangeType;
};

USTRUCT(BlueprintType)
struct FShopItemId : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	uint8 Id;

	void PostReplicatedAdd(const FShopItemIdList& InArraySerializer);

	void PostReplicatedChange(const FShopItemIdList& InArraySerializer);

	void PreReplicatedRemove(const FShopItemIdList& InArraySerializer);

	bool operator==(const FShopItemId& Other) const
	{
		return Id == Other.Id;
	}
};

USTRUCT(BlueprintType)
struct FShopItemIdList : public FFastArraySerializer
{
	GENERATED_BODY()

public:

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams);

	// 인덱스 반환, 없으면 INDEX_NONE 반환
	int32 Contains(uint8 CompareId);

	bool TryAdd(uint8 NewId);

	void Remove(uint8 Id);

	void Modify(uint8 InIndex, uint8 NewId);

	FOnShopItemListChangedDelegate OnShopItemListChangedDelegate;
public:

	UPROPERTY()
	TArray<FShopItemId> IdList;

	UPROPERTY()
	TObjectPtr<AShop> ShopOwner;

	EShopCategoryTab TabType;

public:

	// 유효하지 않으면 INDEX_NONE 반환
	uint8 GetId(uint8 InIndex) const;
};

template<>
struct TStructOpsTypeTraits<FShopItemIdList> : public TStructOpsTypeTraitsBase2<FShopItemIdList>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

#pragma endregion

UCLASS()
class ABYSSDIVERUNDERWORLD_API AShop : public AActor
{
	GENERATED_BODY()
	
public:	

	AShop();

protected:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

#pragma region Methods

public:

	EBuyResult BuyItem(uint8 ItemId, EShopCategoryTab TabType);
	ESellResult SellItem(uint8 ItemId, class AUnitBase* Seller);

	void AddItems(const TArray<uint8>& Ids, EShopCategoryTab TabType);
	void AddItemToList(uint8 ItemId, EShopCategoryTab TabType);
	void RemoveItemToList(uint8 ItemId, EShopCategoryTab TabType);

protected:

	void InitShopWidget();
	void InitData();

private:

	UFUNCTION()
	void OnShopItemListChanged(const FShopItemListChangeInfo& Info);

	bool HasItem(int32 ItemId);
	bool IsItemMeshCached(int32 ItemId);


#pragma endregion

#pragma region Variables

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UDataTable> ItemDataTable; // 테스트용

	UPROPERTY(EditDefaultsOnly, Category = "Shop");
	TArray<uint8> DefaultConsumableItemIdList; // 블루프린트 노출용

	UPROPERTY(EditDefaultsOnly, Category = "Shop");
	TArray<uint8> DefaultEquipmentItemIdList; // 블루프린트 노출용

	UPROPERTY(Replicated)
	FShopItemIdList ShopConsumableItemIdList;

	UPROPERTY(Replicated)
	FShopItemIdList ShopEquipmentItemIdList;

	UPROPERTY(EditDefaultsOnly, Category = "Shop")
	TSubclassOf<UShopWidget> ShopWidgetClass;

	UPROPERTY()
	TObjectPtr<UShopWidget> ShopWidget;

	TArray<FShopItemData> ShopItemDataList;
	
private:

	// ItemId, Mesh류(SM, SKM)
	UPROPERTY()
	TMap<int32, TObjectPtr<UObject>> CachedMeshList;

	TArray<FFADItemDataRow*> DataTableArray;

#pragma endregion
};