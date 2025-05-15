#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "Interface/IADInteractable.h"

#include "Shop.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnShopItemListChangedDelegate, const FShopItemListChangeInfo&);

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
class AUnderwaterCharacter;

struct FFADItemDataRow;
struct FShopItemIdList;

enum class EShopCategoryTab : uint8;
enum class EUpgradeType : uint8;

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
		ShopTab = InTab;
		ShopIndex = InShopIndex;
		ItemIdAfter = InItemId;
		ChangeType = InChangeType;
	}

	EShopCategoryTab ShopTab;
	int16 ShopIndex;
	uint8 ItemIdAfter;
	EShopItemChangeType ChangeType;
};

USTRUCT(BlueprintType)
struct FShopItemId : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	uint8 Id = 0;

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

	// 지워진 인덱스 반환
	int32 Remove(uint8 Id);

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
class ABYSSDIVERUNDERWORLD_API AShop : public AActor, public IIADInteractable
{
	GENERATED_BODY()
	
public:	

	AShop();

protected:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

#pragma region Methods

public:
	virtual void Interact_Implementation(AActor* InstigatorActor) override;

	void OpenShop(AUnderwaterCharacter* Requester);

	UFUNCTION(BlueprintCallable, Category = "Shop")
	void CloseShop(AUnderwaterCharacter* Requester);

	EBuyResult BuyItem(uint8 ItemId, uint8 Quantity, AUnderwaterCharacter* Buyer);
	ESellResult SellItem(uint8 ItemId, AUnderwaterCharacter* Seller);

	void AddItems(const TArray<uint8>& Ids, EShopCategoryTab TabType);
	void AddItemToList(uint8 ItemId, EShopCategoryTab TabType);
	void RemoveItemToList(uint8 ItemId, EShopCategoryTab TabType);

	void InitUpgradeView();

	// 테스트용, 캐릭터의 Interact를 대신함.
	UFUNCTION(BlueprintCallable, Category = "Shop", CallInEditor)
	void Interact_Test(AActor* InstigatorActor);

protected:

	void InitShopWidget();
	void InitData();

private:

	UFUNCTION()
	void OnShopItemListChanged(const FShopItemListChangeInfo& Info);

	UFUNCTION()
	void OnSlotEntryWidgetUpdated(class UShopItemSlotWidget* SlotEntryWidget);

	UFUNCTION()
	void OnSlotEntryClicked(int32 ClickedSlotIndex);

	UFUNCTION()
	void OnBuyButtonClicked(int32 Quantity);

	void OnCloseButtonClicked();
	void OnUpgradeSlotEntryClicked(int32 ClickedSlotIndex);

#pragma endregion

#pragma region Variables

protected:

	UPROPERTY(VisibleAnywhere, Category = "Shop")
	TObjectPtr<UStaticMeshComponent> ShopMeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Shop")
	TObjectPtr<USkeletalMeshComponent> ItemMeshComponent;

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

	UPROPERTY()
	TObjectPtr<class UADInteractableComponent> InteractableComp;

private:

	UPROPERTY()
	TArray<uint8> CachedUpgradeGradeMap;

	int32 CurrentSelectedItemId = INDEX_NONE;
	EUpgradeType CurrentSelectedUpgradeType;
	uint8 bIsOpened : 1;

#pragma endregion

#pragma region Getters, Setters

	virtual UADInteractableComponent* GetInteractableComponent() const override;
	bool HasItem(int32 ItemId);
	bool IsOpened() const;

#pragma endregion

};