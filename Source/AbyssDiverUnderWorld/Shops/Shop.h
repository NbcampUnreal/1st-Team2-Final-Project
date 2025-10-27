#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "Interface/IADInteractable.h"

#include "Shop.generated.h"

#define LOGS(Verbosity, Format, ...) UE_LOG(ShopLog, Verbosity, TEXT("%s(%s) %s"), ANSI_TO_TCHAR(__FUNCTION__), *FString::FromInt(__LINE__), *FString::Printf(Format, ##__VA_ARGS__));

DECLARE_LOG_CATEGORY_EXTERN(ShopLog, Log, All);

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

UENUM()
enum class EDoorState : uint8
{
	Opened,
	Closed,
	Opening,
	Closing
};

enum class ELaunchType
{
	First,
	InProgress,
	Last
};

#pragma endregion

class AShop;
class UShopWidget;
class UShopItemEntryData;
class AUnderwaterCharacter;
class USceneCaptureComponent2D;
class UPointLightComponent;
class ATargetPoint;
class UDataTableSubsystem;
class AADUseItem;

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

	FShopItemListChangeInfo(EShopCategoryTab InTab, int16 InShopIndex, uint8 InItemId, EShopItemChangeType InChangeType, bool bInIsLocked)
	{
		ShopTab = InTab;
		ShopIndex = InShopIndex;
		ItemIdAfter = InItemId;
		ChangeType = InChangeType;
		bIsLocked = bInIsLocked;
	}

	EShopCategoryTab ShopTab;
	int16 ShopIndex;
	uint8 ItemIdAfter;
	EShopItemChangeType ChangeType;
	uint8 bIsLocked : 1;
};

USTRUCT(BlueprintType)
struct FShopItemId : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	uint8 Id = 0;

	UPROPERTY()
	uint8 bIsLocked : 1 = false;

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

	bool TryAdd(uint8 NewId, bool bShouldLock);

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

	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

#pragma region Methods

public:

	virtual void Interact_Implementation(AActor* InstigatorActor) override;

	void OpenShop(AUnderwaterCharacter* Requester);

	UFUNCTION(BlueprintCallable, Category = "Shop")
	void CloseShop(AUnderwaterCharacter* Requester);

	EBuyResult BuyItem(uint8 ItemId, uint8 Quantity, AUnderwaterCharacter* Buyer);
	EBuyResult BuyItems(const TArray<uint8>& ItemIdList, const TArray<int8>& ItemCountList);
	ESellResult SellItem(uint8 ItemId, AUnderwaterCharacter* Seller);

	void AddItems(const TArray<uint8>& Ids, EShopCategoryTab TabType);
	void AddItemToList(uint8 ItemId, EShopCategoryTab TabType, bool bShouldLock);
	void RemoveItemToList(uint8 ItemId, EShopCategoryTab TabType);

	void InitUpgradeView();

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
	void OnSlotEntryDoubleClicked(int32 ClickedSlotIndex);

	UFUNCTION()
	void OnBuyListEntryClicked(int32 ClickedSlotIndex);

	void OnAddButtonClicked(int32 Quantity);
	void OnBuyButtonClicked();
	void OnCloseButtonClicked();
	void OnUpgradeSlotEntryClicked(int32 ClickedSlotIndex);

	// 반환값 : 인덱스, 없으면 INDEX_NONE 
	int8 IsSelectedItem(uint8 ItemId) const;
	void AddToSelectedItemArray(uint8 ItemId, int8 Amount);
	void RemoveFromSelecteItemArray(uint8 ItemId, int8 Amount);
	void RemoveFromSelecteItemArray(int32 BuyListSlotIndex, int8 Amount);

	UDataTableSubsystem* GetDatatableSubsystem();

	// 계산 실패하면 INDEX_NONE 반환
	int32 CalcTotalItemPrice(const TArray<uint8>& ItemIdList, const TArray<int8>& ItemCountList);

	void LaunchItem();
	void RotateDoor(float DegreeFrom, float DegreeTo, float Rate);

	void ClearSelectedInfos();

	EShopCategoryTab GetCurrentTab();

#pragma endregion

#pragma region Variables

protected:

	UPROPERTY(VisibleAnywhere, Category = "Shop")
	TObjectPtr<UStaticMeshComponent> ShopMeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Shop")
	TObjectPtr<USkeletalMeshComponent> ShopMerchantMeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Shop")
	TObjectPtr<USkeletalMeshComponent> ItemMeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Shop")
	TObjectPtr<USceneCaptureComponent2D> ItemMeshCaptureComp;

	UPROPERTY(VisibleAnywhere, Category = "Shop")
	TObjectPtr<UPointLightComponent> LightComp;

	UPROPERTY(EditDefaultsOnly, Category = "ShopSettings|ConsumableItem")
	TArray<uint8> DefaultConsumableItemIdList; // 블루프린트 노출용 판매할 소비 아이템 리스트

	UPROPERTY(EditDefaultsOnly, Category = "ShopSettings|ConsumableItem")
	TArray<bool> ConsumableItemLockedStates; // 소비 아이템 잠금 여부, true이면 구매 불가능하도록 작동한다.

	UPROPERTY(EditDefaultsOnly, Category = "ShopSettings|EquipmentItem")
	TArray<uint8> DefaultEquipmentItemIdList; // 블루프린트 노출용 판매할 장비 아이템 리스트

	UPROPERTY(EditDefaultsOnly, Category = "ShopSettings|EquipmentItem")
	TArray<bool> EquipmentItemLockedStates; // 장비 아이템 잠금 여부, true이면 구매 불가능하도록 작동한다.
	
	UPROPERTY(EditInstanceOnly, Category = "ShopSettings")
	TObjectPtr<ATargetPoint> OriginPoint;

	UPROPERTY(EditInstanceOnly, Category = "ShopSettings")
	TObjectPtr<ATargetPoint> DestinationPoint;

	UPROPERTY(EditInstanceOnly, Category = "ShopSettings")
	float ForceAmount = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "ShopSettings")
	float LaunchItemInterval = 1.0f;

	UPROPERTY(EditAnywhere, Category = "ShopSettings")
	float LaunchItemIntervalAtFirst = 1.0f;

	UPROPERTY(EditAnywhere, Category = "ShopSettings")
	float LaunchItemIntervalAtLast = 1.0f;

	UPROPERTY(EditAnywhere, Category = "ShopSettings")
	float ErrorOfLaunchDirection = 1.0f;

	UPROPERTY(EditAnywhere, Category = "ShopSettings | Door")
	float DoorOpenSpeed = 1.0f;

	UPROPERTY(EditAnywhere, Category = "ShopSettings | Door")
	float DoorCloseSpeed = 1.0f;

	UPROPERTY(EditAnywhere, Category = "ShopSettings | Door")
	float DesiredOpenDegree = 1.0f;

	UPROPERTY(EditAnywhere, Category = "ShopSettings | Door")
	float DesiredCloseDegree = 1.0f;

	UPROPERTY(EditInstanceOnly, Category = "ShopSettings | Door")
	TObjectPtr<AActor> DoorActor;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	uint8 bIsHold : 1;

private:

	TArray<uint8> CachedUpgradeGradeMap;

	int32 CurrentSelectedItemId = INDEX_NONE;
	EUpgradeType CurrentSelectedUpgradeType;
	uint8 bIsOpened : 1;

	// ItemId, ItemCount
	TArray<uint8> SelectedItemIdArray;
	TArray<int8> SelectedItemCountArray;

	static const int8 MaxItemCount;

	TQueue<uint8> ReadyQueueForLaunchItemById;

	float ElapsedTime = 0.0f;

	int32 TotalPriceOfBuyList = 0;

	ELaunchType CurrentLaunchType = ELaunchType::First;


	UPROPERTY(Replicated)
	EDoorState CurrentDoorState = EDoorState::Closed;

	float CurrentDoorRate = 0.0f;

	uint8 bIsDoorOpenSoundPlayed : 1 = false;
	uint8 bIsDoorCloseSoundPlayed : 1 = false;
	int32 DoorOpenAudioId = INDEX_NONE;
	int32 DoorCloseAudioId = INDEX_NONE;

#pragma endregion

#pragma region Getters, Setters

public:

	virtual UADInteractableComponent* GetInteractableComponent() const override;
	virtual bool IsHoldMode() const;
	bool HasItem(int32 ItemId);
	bool IsOpened() const;
	virtual FString GetInteractionDescription() const override;

private:

	class USoundSubsystem* GetSoundSubsystem();

#pragma endregion

};