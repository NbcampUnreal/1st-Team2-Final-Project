#include "Shops/Shop.h"

#include "Character/UnitBase.h"
#include "Character/UnderwaterCharacter.h"
#include "Character/UpgradeComponent.h"

#include "Shops/ShopWidgets/ShopCategoryTabWidget.h"
#include "Shops/ShopWidgets/ShopWidget.h"
#include "Shops/ShopItemEntryData.h"
#include "Shops/ShopWidgets/ShopElementInfoWidget.h"
#include "Shops/ShopWidgets/ShopItemSlotWidget.h"
#include "ShopInteractionComponent.h"

#include "AbyssDiverUnderWorld.h"	
#include "Inventory/ADInventoryComponent.h"
#include "Subsystems/DataTableSubsystem.h"

#include "Framework/ADPlayerState.h"
#include "Framework/ADInGameState.h"

#include "DataRow/UpgradeDataRow.h"
#include "DataRow/FADItemDataRow.h"

#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/PointLightComponent.h"

DEFINE_LOG_CATEGORY(ShopLog);

#pragma region FShopItemId

void FShopItemId::PostReplicatedAdd(const FShopItemIdList& InArraySerializer)
{
	LOGS(Warning, TEXT("Shop item added: Id = %d"), Id);
	int32 Index = InArraySerializer.IdList.IndexOfByKey(*this);
	FShopItemListChangeInfo Info(InArraySerializer.TabType, Index, Id, EShopItemChangeType::Added);

	InArraySerializer.OnShopItemListChangedDelegate.Broadcast(Info);
}

void FShopItemId::PostReplicatedChange(const FShopItemIdList& InArraySerializer)
{
	LOGS(Warning, TEXT("Shop item changed: Id = %d"), Id);

	int32 Index = InArraySerializer.IdList.IndexOfByKey(*this);
	FShopItemListChangeInfo Info(InArraySerializer.TabType, Index, Id, EShopItemChangeType::Modified);

	InArraySerializer.OnShopItemListChangedDelegate.Broadcast(Info);
}

void FShopItemId::PreReplicatedRemove(const FShopItemIdList& InArraySerializer)
{
	LOGS(Warning, TEXT("Shop item removed: Id = %d"), Id);

	int32 Index = InArraySerializer.IdList.IndexOfByKey(*this);
	FShopItemListChangeInfo Info(InArraySerializer.TabType, Index, Id, EShopItemChangeType::Removed);

	InArraySerializer.OnShopItemListChangedDelegate.Broadcast(Info);
}

#pragma endregion

#pragma region FShopItemIdList

bool FShopItemIdList::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FShopItemId, FShopItemIdList>(IdList, DeltaParams, *this);
}

int32 FShopItemIdList::Contains(uint8 CompareId)
{
	int32 ElementCount = IdList.Num();
	for (int32 i = 0; i < ElementCount; ++i)
	{
		if (IdList[i].Id == CompareId)
		{
			return i;
		}
	}

	return INDEX_NONE;
}

bool FShopItemIdList::TryAdd(uint8 NewId)
{
	if (Contains(NewId) != INDEX_NONE)
	{
		LOGS(Warning, TEXT("You Trying to Add Exist Id : %d"), NewId);
		return false;
	}

	FShopItemId ShopId;
	ShopId.Id = NewId;

	IdList.Add(ShopId);
	MarkItemDirty(IdList.Last());

	return true;
}

int32 FShopItemIdList::Remove(uint8 Id)
{
	int32 Index = Contains(Id);
	if (Index == INDEX_NONE)
	{
		LOGS(Warning, TEXT("You Trying to Remove Not Exist Id : %d"), Id);
		return INDEX_NONE;
	}

	IdList.RemoveAt(Index);
	MarkArrayDirty();
	return Index;
}

void FShopItemIdList::Modify(uint8 InIndex, uint8 NewId)
{
	if (IdList.IsValidIndex(InIndex) == false)
	{
		LOGS(Warning, TEXT("You Trying to Modify From Not Valid Index : %d"), InIndex);
		return;
	}

	FShopItemId& ItemId = IdList[InIndex];
	ItemId.Id = NewId;
	MarkItemDirty(ItemId);
}

uint8 FShopItemIdList::GetId(uint8 InIndex) const
{
	if (IdList.IsValidIndex(InIndex) == false)
	{
		LOGS(Warning, TEXT("You Trying to Get From Not Valid Index"));
		return INDEX_NONE;
	}

	return IdList[InIndex].Id;
}
#pragma endregion

AShop::AShop()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	ShopMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShopMesh"));
	SetRootComponent(ShopMeshComponent);
	ShopMeshComponent->SetMobility(EComponentMobility::Static);

	ItemMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh"));
	ItemMeshComponent->SetupAttachment(RootComponent);
	ItemMeshComponent->SetVisibleInSceneCaptureOnly(true);
	ItemMeshComponent->SetIsReplicated(false);
	ItemMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 라이팅 채널 2번만 사용. 상점 아이템 메쉬용
	ItemMeshComponent->LightingChannels.bChannel0 = false;
	ItemMeshComponent->LightingChannels.bChannel1 = false;
	ItemMeshComponent->LightingChannels.bChannel2 = true;

	// 설정을 좀 더 건드려야 함. 
	ItemMeshCaptureComp = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("ItemMeshCapureComp"));
	ItemMeshCaptureComp->SetupAttachment(RootComponent);
	ItemMeshCaptureComp->ShowFlags.AntiAliasing = true;
	ItemMeshCaptureComp->ShowFlags.Atmosphere = false;
	ItemMeshCaptureComp->ShowFlags.BSP = false;
	ItemMeshCaptureComp->ShowFlags.Cloud = false;
	ItemMeshCaptureComp->ShowFlags.Decals = false;
	ItemMeshCaptureComp->ShowFlags.Fog = false;
	ItemMeshCaptureComp->ShowFlags.Landscape = false;
	ItemMeshCaptureComp->ShowFlags.Particles = false;
	ItemMeshCaptureComp->ShowFlags.SkeletalMeshes = true;
	ItemMeshCaptureComp->ShowFlags.StaticMeshes = false;
	ItemMeshCaptureComp->ShowFlags.Translucency = false;

	ItemMeshCaptureComp->ShowFlags.InstancedFoliage = false;
	ItemMeshCaptureComp->ShowFlags.InstancedGrass = false;
	ItemMeshCaptureComp->ShowFlags.InstancedStaticMeshes = false;
	ItemMeshCaptureComp->ShowFlags.Paper2DSprites = false;
	ItemMeshCaptureComp->ShowFlags.TextRender = false;
	ItemMeshCaptureComp->ShowFlags.TemporalAA = true;

	ItemMeshCaptureComp->ShowFlags.Bloom = true;
	ItemMeshCaptureComp->ShowFlags.EyeAdaptation = false;
	ItemMeshCaptureComp->ShowFlags.LocalExposure = false;
	ItemMeshCaptureComp->ShowFlags.MotionBlur = false;
	ItemMeshCaptureComp->ShowFlags.PostProcessMaterial = true;
	ItemMeshCaptureComp->ShowFlags.ToneCurve = true;
	ItemMeshCaptureComp->ShowFlags.Tonemapper = true;

	ItemMeshCaptureComp->ShowFlags.SkyLighting = false;

	ItemMeshCaptureComp->ShowFlags.AmbientOcclusion = false;
	ItemMeshCaptureComp->ShowFlags.DynamicShadows = false;

	ItemMeshCaptureComp->ShowFlags.AmbientCubemap = false;
	ItemMeshCaptureComp->ShowFlags.DistanceFieldAO = false;
	ItemMeshCaptureComp->ShowFlags.LightFunctions = false;
	ItemMeshCaptureComp->ShowFlags.LightShafts = false;
	ItemMeshCaptureComp->ShowFlags.ReflectionEnvironment = false;
	ItemMeshCaptureComp->ShowFlags.ScreenSpaceReflections = false;
	ItemMeshCaptureComp->ShowFlags.TexturedLightProfiles = false;
	ItemMeshCaptureComp->ShowFlags.VolumetricFog = false;

	ItemMeshCaptureComp->ShowFlags.NaniteMeshes = true;

	ItemMeshCaptureComp->bCaptureEveryFrame = false;

	ItemMeshCaptureComp->SetRelativeLocation(FVector(200, 0, 0));

	LightComp = CreateDefaultSubobject<UPointLightComponent>(TEXT("LightComp"));
	LightComp->SetupAttachment(ItemMeshCaptureComp);

	// 라이팅 채널 2번만 사용. 상점 아이템 메쉬용
	LightComp->LightingChannels.bChannel0 = false;
	LightComp->LightingChannels.bChannel1 = false;
	LightComp->LightingChannels.bChannel2 = true;

	LightComp->SetMobility(EComponentMobility::Stationary);
	LightComp->SetRelativeLocation(FVector(0, 0, 100));

	InteractableComp = CreateDefaultSubobject<UADInteractableComponent>(TEXT("InteractableComp"));

	bIsOpened = false;
	CurrentSelectedUpgradeType = EUpgradeType::Max;
	bIsHold = false;
}

void AShop::PostInitializeComponents()
{
	Super::PostInitializeComponents();

#if WITH_EDITOR

	// 게임 중이 아닌 경우 리턴(블루프린트 상일 경우)
	// PostInitializeComponents는 블루프린트에서도 발동함
	UWorld* World = GetWorld();
	if (World == nullptr || World->IsGameWorld() == false)
	{
		return;
	}

#endif

	InitShopWidget();
	InitData();
}

void AShop::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShop, ShopConsumableItemIdList);
	DOREPLIFETIME(AShop, ShopEquipmentItemIdList);
}

void AShop::BeginPlay()
{
	Super::BeginPlay();

	AADInGameState* GS = Cast<AADInGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (ensureMsgf(GS, TEXT("올바른 GS 타입이 아닌 듯. 게임모드의 GameState가 ADInGameState가 맞는지 확인 필요.")) == false)
	{
		return;
	}

	GS->TeamCreditsChangedDelegate.AddUObject(ShopWidget, &UShopWidget::SetTeamMoneyText);
}

void AShop::Interact_Implementation(AActor* InstigatorActor)
{
	if (HasAuthority() == false)
	{
		return;
	}

	ACharacter* InteractingCharacter = Cast<ACharacter>(InstigatorActor);
	if (InteractingCharacter == nullptr)
	{
		return;
	}

	// 캐릭터로부터 컴포넌트 Get, 나중에 Getter로 가져옴
	UShopInteractionComponent* ShopInteractionComp = InteractingCharacter->FindComponentByClass<UShopInteractionComponent>();
	if (ShopInteractionComp == nullptr)
	{
		LOGS(Warning, TEXT("ShopInteractionComp == nullptr"));
		return;
	}

	ShopInteractionComp->SetCurrentInteractingShop(this);
	ShopInteractionComp->C_OpenShop(this);
}

void AShop::OpenShop(AUnderwaterCharacter* Requester)
{
	if (Requester->IsLocallyControlled() == false)
	{
		return;
	}

	if (bIsOpened)
	{
		return;
	}

	InitUpgradeView();
	ShopWidget->AddToViewport();

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	PC->SetInputMode(FInputModeUIOnly());
	PC->SetShowMouseCursor(true);
	bIsOpened = true;
	PC->SetIgnoreMoveInput(true);
	ItemMeshCaptureComp->bCaptureEveryFrame = true;
}

void AShop::CloseShop(AUnderwaterCharacter* Requester)
{
	if (Requester->IsLocallyControlled() == false)
	{
		return;
	}

	if (bIsOpened == false)
	{
		return;
	}
	ShopWidget->PlayCloseAnimation();

	FTimerHandle RemoveWidgetTimerHandle;
	float RemoveDelay = ShopWidget->GetCloseShopAnimEndTime();
	GetWorld()->GetTimerManager().SetTimer(RemoveWidgetTimerHandle,
		FTimerDelegate::CreateLambda([this]() 
			{ 
				ShopWidget->RemoveFromParent();

				APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
				PC->SetInputMode(FInputModeGameOnly());
				PC->SetShowMouseCursor(false);
				bIsOpened = false;
				PC->SetIgnoreMoveInput(false);

				ItemMeshCaptureComp->bCaptureEveryFrame = false;

			}), RemoveDelay, false);

}

EBuyResult AShop::BuyItem(uint8 ItemId, uint8 Quantity, AUnderwaterCharacter* Buyer)
{
	if (HasAuthority() == false)
	{
		LOGS(Error, TEXT("Has No Authority"));
		return EBuyResult::HasNoAuthority;
	}

	if (HasItem(ItemId) == false)
	{
		return EBuyResult::NotExistItem;
	}

	if (Quantity <= 0)
	{
		LOGS(Warning, TEXT("Quantity <= 0"));
		return EBuyResult::FailedFromOtherReason;
	}
	
	AADPlayerState* PS = Cast<AADPlayerState>(Buyer->GetPlayerState());
	if (PS == nullptr)
	{
		LOGS(Error, TEXT("PS == nullptr"));
		return EBuyResult::FailedFromOtherReason;
	}

	FFADItemDataRow* ItemDataRow = GetGameInstance()->GetSubsystem<UDataTableSubsystem>()->GetItemData(ItemId);
	if (ItemDataRow == nullptr)
	{
		LOGS(Error, TEXT("ItemData == nullptr"));
		return EBuyResult::FailedFromOtherReason;
	}

	AADInGameState* GS = Cast<AADInGameState>(UGameplayStatics::GetGameState(GetWorld()));
	check(GS);

	int32 TeamCredits = GS->GetTotalTeamCredit();
	if (TeamCredits < ItemDataRow->Price * Quantity)
	{
		// 돈부족 효과 재생
		return EBuyResult::NotEnoughMoney;
	}

	GS->SetTotalTeamCredit(TeamCredits - ItemDataRow->Price * Quantity);

	FItemData ItemData;
	ItemData.Amount = ItemDataRow->Amount;
	ItemData.Id = ItemDataRow->Id;
	ItemData.ItemType = ItemDataRow->ItemType;
	ItemData.Mass = ItemDataRow->Weight;
	ItemData.Name = ItemDataRow->Name;
	ItemData.Price = ItemDataRow->Price;
	ItemData.Quantity = Quantity;
	ItemData.Thumbnail = ItemDataRow->Thumbnail;

	PS->GetInventory()->AddInventoryItem(ItemData);

	LOGS(Log, TEXT("Buying Item Succeeded : %s"), *ItemDataRow->Name.ToString());
	return EBuyResult::Succeeded;
}

ESellResult AShop::SellItem(uint8 ItemId, AUnderwaterCharacter* Seller)
{
	if (HasAuthority() == false)
	{
		LOGS(Log, TEXT("Has No Authority"));
		return ESellResult::HasNoAuthority;
	}

	/*if (아이템 안 갖고 있으면)
	{
		return ESellResult::NotExistItem;
	}*/

	// 돈 추가
	// 인벤토리에서 아이템 삭제

	return ESellResult::Succeeded;
}

void AShop::AddItems(const TArray<uint8>& Ids, EShopCategoryTab TabType)
{
	if (HasAuthority() == false)
	{
		LOGS(Log, TEXT("Has No Authority"));
		return;
	}

	for (auto& Id : Ids)
	{
		AddItemToList(Id, TabType);
	}
}

void AShop::AddItemToList(uint8 ItemId, EShopCategoryTab TabType)
{
	if (HasAuthority() == false)
	{
		LOGS(Log, TEXT("Has No Authority"));
		return;
	}

	if (TabType >= EShopCategoryTab::Max)
	{
		LOGS(Error, TEXT("Weird Category Type : %d"), TabType);
		return;
	}
	LOGS(Warning, TEXT("Adding ~~ ITemId : %d, TabType : %d"), ItemId, TabType);

	bool bIsAddSucceeded = false;
	int32 SlotIndex;
	switch (TabType)
	{
	case EShopCategoryTab::Consumable:
		bIsAddSucceeded = ShopConsumableItemIdList.TryAdd(ItemId);
		SlotIndex = ShopConsumableItemIdList.IdList.Num() - 1;
		break;
	case EShopCategoryTab::Equipment:
		bIsAddSucceeded = ShopEquipmentItemIdList.TryAdd(ItemId);
		SlotIndex = ShopEquipmentItemIdList.IdList.Num() - 1;
		break;
	case EShopCategoryTab::Upgrade:
		LOGS(Error, TEXT("Can't Add Item to Upgrade Tab"));
		return;
	case EShopCategoryTab::Max:
		LOGS(Warning, TEXT("Weird CategoryTab(Max).. ItemId : %d, TabType : %d"), ItemId, TabType);
		check(false);
		return;
	default:
		LOGS(Warning, TEXT("Weird CategoryTab.. ItemId : %d, TabType : %d"), ItemId, TabType);
		check(false);
		return;
	}

	if (bIsAddSucceeded == false)
	{
		LOGS(Warning, TEXT("Add Failed, ItemId : %d, TabType : %d"), ItemId, TabType);
		return;
	}

	FFADItemDataRow* ItemDataRow = GetGameInstance()->GetSubsystem<UDataTableSubsystem>()->GetItemData(ItemId);
	if (ItemDataRow == nullptr)
	{
		LOGS(Error, TEXT("ItemData == nullptr"));
		return;
	}

	UShopItemEntryData* EntryData = NewObject<UShopItemEntryData>();
	EntryData->Init(SlotIndex, ItemDataRow->Thumbnail, ItemDataRow->Description); // 임시
	EntryData->OnEntryUpdatedFromDataDelegate.AddUObject(this, &AShop::OnSlotEntryWidgetUpdated);

	ShopWidget->AddItem(EntryData, TabType);

	if (TabType == EShopCategoryTab::Consumable)
	{
		LOGS(Warning, TEXT("ItemAdded to Consumable Tab, Index : %d"), SlotIndex);
	}
	else
	{
		LOGS(Warning, TEXT("ItemAdded to Equipment Tab, Index : %d"), SlotIndex);
	}
	
}

void AShop::RemoveItemToList(uint8 ItemId, EShopCategoryTab TabType)
{
	if (HasAuthority() == false)
	{
		LOGS(Log, TEXT("Has No Authority"));
		return;
	}

	if (TabType >= EShopCategoryTab::Max)
	{
		LOGS(Error, TEXT("Weird Category Type : %d"), TabType);
		return;
	}

	int32 RemovedIndex = INDEX_NONE;

	switch (TabType)
	{
	case EShopCategoryTab::Consumable:
		RemovedIndex = ShopConsumableItemIdList.Remove(ItemId);
		break;
	case EShopCategoryTab::Equipment:
		RemovedIndex = ShopEquipmentItemIdList.Remove(ItemId);
		break;
	case EShopCategoryTab::Upgrade:
		LOGS(Error, TEXT("Can't Remove Item From Upgrade Tab"));
		break;
	case EShopCategoryTab::Max:
		check(false);
		break;
	default:
		check(false);
		break;
	}

	if (RemovedIndex == INDEX_NONE)
	{
		return;
	}

	ShopWidget->RemoveItem(RemovedIndex, TabType);
}

void AShop::InitUpgradeView()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	AADPlayerState* PS = PC->GetPlayerState<AADPlayerState>();
	if (PS == nullptr)
	{
		LOGS(Error, TEXT("PS == nullptr"));
		return;
	}

	// 나중에 Get으로 대체
	UUpgradeComponent* UpgradeComp = PS->GetUpgradeComp();
	if (UpgradeComp == nullptr)
	{
		LOGS(Error, TEXT("UpgradeComp == nullptr"));
		return;
	}

	UEnum* UpgradeTypeEnum = StaticEnum<EUpgradeType>();
	if (UpgradeTypeEnum == nullptr)
	{
		LOGS(Error, TEXT("Cant Find Enum"));
		return;
	}

	UDataTableSubsystem* DataTableSubsystem = GetGameInstance()->GetSubsystem<UDataTableSubsystem>();
	if (DataTableSubsystem == nullptr)
	{
		LOGS(Error, TEXT("DataTableSubsystem == nullptr"));
		return;
	}

	// 마지막 값은 항상 EUpgradeType_MAX이 자동생성된다고 함.(선언한 MAX를 말하는게 아님), 그리고 내가 선언한 MAX도 제외
#if WITH_EDITOR
	int32 EnumCount = UpgradeTypeEnum->NumEnums() - 2;
#else
	// 패키징에서는 마지막값 자동 생성 안되네..
	int32 EnumCount = UpgradeTypeEnum->NumEnums() - 1;
#endif

	for (int32 i = 0; i < UpgradeTypeEnum->NumEnums(); ++i)
	{
		int64 Value = UpgradeTypeEnum->GetValueByIndex(i);
		FString Name = UpgradeTypeEnum->GetNameStringByIndex(i);

		LOGS(Warning, TEXT("Enum %d: %s"), Value, *Name);
	}

	if (CachedUpgradeGradeMap.Num() != EnumCount)
	{
		CachedUpgradeGradeMap.Init(-1, EnumCount);
	}

	bool bHasChanged = false;

	TArray<TObjectPtr<UShopItemEntryData>>& UpgradeTabEntryDataList = ShopWidget->GetUpgradeTabEntryDataList();
	if (UpgradeTabEntryDataList.Num() <= EnumCount)
	{
		UpgradeTabEntryDataList.SetNum(EnumCount);
	}

	LOGS(Error, TEXT("Enumcount : %d"), EnumCount);
	for (int32 i = 0; i < EnumCount; ++i)
	{
		EUpgradeType UpgradeType = EUpgradeType(i);
		uint8 Grade = UpgradeComp->GetCurrentGrade(UpgradeType);

		if (Grade == 0)
		{
			LOGS(Error, TEXT("Weird Grade Detected, Type : %d"), UpgradeType);
			return;
		}

		if (CachedUpgradeGradeMap[i] == Grade)
		{
			LOGS(Warning, TEXT("UpgradeState : UpgradeType(%d) - Grade(%d)"), UpgradeType, Grade);
			continue;
		}
		LOGS(Warning, TEXT("UpgradeState(Renewed) : UpgradeType(%d) - Grade(%d)"), UpgradeType, Grade);
		CachedUpgradeGradeMap[i] = Grade;

		FUpgradeDataRow* UpgradeDataRow = DataTableSubsystem->GetUpgradeData(UpgradeType, Grade);

		UShopItemEntryData* UpgradeEntryData = NewObject<UShopItemEntryData>();
		UpgradeEntryData->Init(i, nullptr, FString(TEXT("Temp Tooltip")));
		UpgradeEntryData->OnEntryUpdatedFromDataDelegate.AddUObject(this, &AShop::OnSlotEntryWidgetUpdated);
		UpgradeTabEntryDataList[i] = UpgradeEntryData;

		bHasChanged = true;
	}

	if (bHasChanged == false)
	{
		return;
	}

	if (ShopWidget->GetCurrentActivatedTab() != EShopCategoryTab::Upgrade)
	{
		return;
	}

	ShopWidget->RefreshItemView();
	OnUpgradeSlotEntryClicked(int32(CurrentSelectedUpgradeType));
}

void AShop::Interact_Test(AActor* InstigatorActor)
{
	InteractableComp->Interact(InstigatorActor);
}

void AShop::InitShopWidget()
{
	LOGS(Warning, TEXT("Init Shop Widget Start"));
	ShopConsumableItemIdList.IdList.Empty();
	ShopConsumableItemIdList.OnShopItemListChangedDelegate.AddUObject(this, &AShop::OnShopItemListChanged);
	ShopConsumableItemIdList.ShopOwner = this;
	ShopConsumableItemIdList.TabType = EShopCategoryTab::Consumable;

	ShopEquipmentItemIdList.IdList.Empty();
	ShopEquipmentItemIdList.OnShopItemListChangedDelegate.AddUObject(this, &AShop::OnShopItemListChanged);
	ShopEquipmentItemIdList.ShopOwner = this;
	ShopEquipmentItemIdList.TabType = EShopCategoryTab::Equipment;

	ShopWidget = CreateWidget<UShopWidget>(GetWorld(), ShopWidgetClass, FName(TEXT("ShopWidget")));
	check(ShopWidget);

	ShopWidget->SetCurrentActivatedTab(EShopCategoryTab::Equipment);
	ShopWidget->OnShopCloseButtonClickedDelegate.AddUObject(this, &AShop::OnCloseButtonClicked);

	UShopElementInfoWidget* InfoWidget = ShopWidget->GetInfoWidget();
	check(InfoWidget);

	InfoWidget->Init(ItemMeshComponent);
	InfoWidget->OnBuyButtonClickedDelegate.AddUObject(this, &AShop::OnBuyButtonClicked);

	ItemMeshCaptureComp->ShowOnlyActorComponents(this);
	ensureMsgf(ItemMeshCaptureComp->TextureTarget, TEXT("상점에 있는 ItemMeshCaptureComp의 TextureTarget이 지정이 되어있지 않습니다"));
}

void AShop::InitData()
{
	if (HasAuthority() == false)
	{
		return;
	}

	ShopConsumableItemIdList.MarkArrayDirty();

	for (const auto& Id : DefaultConsumableItemIdList)
	{
		AddItemToList(Id, EShopCategoryTab::Consumable);
		LOGS(Warning, TEXT("Adding ConsumableItem, Id : %d "), Id);
	}

	ShopEquipmentItemIdList.MarkArrayDirty();

	for (const auto& Id : DefaultEquipmentItemIdList)
	{
		AddItemToList(Id, EShopCategoryTab::Equipment);
		LOGS(Warning, TEXT("Adding Equipment, Id : %d "), Id);
	}

	ShopWidget->ShowItemViewForTab(EShopCategoryTab::Equipment);
}

void AShop::OnShopItemListChanged(const FShopItemListChangeInfo& Info)
{
	LOGS(Warning, TEXT("Begin Change"));

	if (Info.ChangeType >= EShopItemChangeType::Max)
	{
		LOGS(Error, TEXT("Weird Change Type : %d"), Info.ChangeType);
		return;
	}

	if (Info.ShopTab >= EShopCategoryTab::Upgrade)
	{
		LOGS(Error, TEXT("Tab.. Weird.. : %d"), Info.ChangeType);
		return;
	}

	UShopItemEntryData* EntryData = nullptr;
	FFADItemDataRow* ItemDataRow = GetGameInstance()->GetSubsystem<UDataTableSubsystem>()->GetItemData(Info.ItemIdAfter);
	if (ItemDataRow == nullptr)
	{
		LOGS(Error, TEXT("ItemData == nullptr"));
		return;
	}

	switch (Info.ChangeType)
	{
	case EShopItemChangeType::Added:

		EntryData = NewObject<UShopItemEntryData>();
		EntryData->Init(Info.ShopIndex, ItemDataRow->Thumbnail, ItemDataRow->Description); // 임시
		EntryData->OnEntryUpdatedFromDataDelegate.AddUObject(this, &AShop::OnSlotEntryWidgetUpdated);
		
		ShopWidget->AddItem(EntryData, Info.ShopTab);
		LOGS(Warning, TEXT("Add Data End, Category : %d"), ItemDataRow->ItemType);
		break;
	case EShopItemChangeType::Removed:
		ShopWidget->RemoveItem(Info.ShopIndex, Info.ShopTab);
		break;
	case EShopItemChangeType::Modified:
		ShopWidget->ModifyItem(Info.ShopIndex, ItemDataRow->Thumbnail, ItemDataRow->Description, Info.ShopTab);
		break;
	case EShopItemChangeType::Max:
		check(false);
		break;
	default:
		check(false);
		break;
	}

	ShopWidget->RefreshItemView();
}

void AShop::OnSlotEntryWidgetUpdated(UShopItemSlotWidget* SlotEntryWidget)
{
	SlotEntryWidget->OnShopItemSlotWidgetClickedDelegate.BindUObject(this, &AShop::OnSlotEntryClicked);
}

void AShop::OnSlotEntryClicked(int32 ClickedSlotIndex)
{
	EShopCategoryTab CurrentTab = ShopWidget->GetCurrentActivatedTab();

	if (CurrentTab >= EShopCategoryTab::Max)
	{
		LOGS(Error, TEXT("Weird Tab Type : %d"), CurrentTab);
		return;
	}

	int32 ItemId;

	switch (CurrentTab)
	{
	case EShopCategoryTab::Consumable:
		ItemId = ShopConsumableItemIdList.IdList[ClickedSlotIndex].Id;
		break;
	case EShopCategoryTab::Equipment:
		ItemId = ShopEquipmentItemIdList.IdList[ClickedSlotIndex].Id;
		break;
	case EShopCategoryTab::Upgrade:
		LOGS(Error, TEXT("UpgradeSlot Click!, Index : %d"), ClickedSlotIndex);
		OnUpgradeSlotEntryClicked(ClickedSlotIndex);
		return;
	case EShopCategoryTab::Max:
		check(false);
		return;
	default:
		check(false);
		return;
	}

	FFADItemDataRow* ItemDataRow = GetGameInstance()->GetSubsystem<UDataTableSubsystem>()->GetItemData(ItemId);
	if (ItemDataRow == nullptr)
	{
		LOGS(Error, TEXT("ItemData == nullptr"));
		return;
	}

	USkeletalMesh* ItemMesh = ItemDataRow->SkeletalMesh;

	ShopWidget->ShowItemInfos(ItemId);
	LOGS(Log, TEXT("Showing Item Infos..., id : %d"), ItemId);
	CurrentSelectedItemId = ItemId;
}

void AShop::OnBuyButtonClicked(int32 Quantity)
{
	AADInGameState* GS = CastChecked<AADInGameState>(UGameplayStatics::GetGameState(GetWorld()));
	UDataTableSubsystem* DataTableSubsystem = GetGameInstance()->GetSubsystem<UDataTableSubsystem>();

	int32 TotalTeamCredit = GS->GetTotalTeamCredit();

	EShopCategoryTab CurrentTab = ShopWidget->GetCurrentActivatedTab();

	if (CurrentTab == EShopCategoryTab::Upgrade)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		AADPlayerState* PS = PC->GetPlayerState<AADPlayerState>();
		if (PS == nullptr)
		{
			LOGS(Error, TEXT("PS == nullptr"));
			return;
		}

		UUpgradeComponent* UpgradeComp = PS->GetUpgradeComp();
		if (UpgradeComp == nullptr)
		{
			LOGS(Error, TEXT("UpgradeComp == nullptr"));
			return;
		}

		bool bIsMaxGrade = UpgradeComp->IsMaxGrade(CurrentSelectedUpgradeType);
		if (bIsMaxGrade)
		{
			LOGS(Log, TEXT("This Upgrade Type Has Reached to Max Level"));
			return;
		}

		int32 Grade = UpgradeComp->GetCurrentGrade(CurrentSelectedUpgradeType);
		

		FUpgradeDataRow* UpgradeData = DataTableSubsystem->GetUpgradeData(CurrentSelectedUpgradeType, Grade + 1);

		if (TotalTeamCredit < UpgradeData->Price)
		{
			LOGS(Warning, TEXT("업그레이드 돈부족!! 남은 돈 : %d, 필요한 돈 : %d"), TotalTeamCredit, UpgradeData->Price);
			return;
		}
		
		UpgradeComp->S_RequestUpgrade(CurrentSelectedUpgradeType);
		return;
	}

	AUnderwaterCharacter* BuyingCharacter = Cast<AUnderwaterCharacter>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetPawn());
	if (BuyingCharacter == nullptr)
	{
		LOGS(Warning, TEXT("BuyingCharacter == nullptr"));
		return;
	}

	UShopInteractionComponent* ShopInteractionComp = BuyingCharacter->GetShopInteractionComponent();
	if (ShopInteractionComp == nullptr)
	{
		LOGS(Warning, TEXT("ShopInteractionComp == nullptr"));
		return;
	}

	FFADItemDataRow* ItemData = DataTableSubsystem->GetItemData(CurrentSelectedItemId);

	if (TotalTeamCredit < ItemData->Price * Quantity)
	{
		LOGS(Warning, TEXT("아이템 구매 돈부족!! 남은 돈 : %d, 필요한 돈 : %d"), TotalTeamCredit, ItemData->Price * Quantity);
		return;
	}

	ShopInteractionComp->S_RequestBuyItem(CurrentSelectedItemId, Quantity);
}

void AShop::OnCloseButtonClicked()
{
	AUnderwaterCharacter* Requester = Cast<AUnderwaterCharacter>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetPawn());
	if (Requester == nullptr)
	{
		LOGS(Error, TEXT("Requester == nullptr"));
		return;
	}

	CloseShop(Requester);
}

void AShop::OnUpgradeSlotEntryClicked(int32 ClickedSlotIndex)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	AADPlayerState* PS = PC->GetPlayerState<AADPlayerState>();
	if (PS == nullptr)
	{
		LOGS(Error, TEXT("PS == nullptr"));
		return;
	}

	UUpgradeComponent* UpgradeComp = PS->GetUpgradeComp();
	if (UpgradeComp == nullptr)
	{
		LOGS(Error, TEXT("UpgradeComp == nullptr"));
		return;
	}

	EUpgradeType UpgradeType = EUpgradeType(ClickedSlotIndex);
	
	uint8 CurrentGrade = UpgradeComp->GetCurrentGrade(UpgradeType);
	if (CurrentGrade == 0)
	{
		LOGS(Log, TEXT("Weird Upgrade Type Detected : %d"), UpgradeType);
		return;
	}

	UDataTableSubsystem* DataTableSubSystem = GetGameInstance()->GetSubsystem<UDataTableSubsystem>();
	if (DataTableSubSystem == nullptr)
	{
		LOGS(Error, TEXT("DataTableSubSystem == nullptr"));
		return;
	}

	bool bIsMaxGrade = UpgradeComp->IsMaxGrade(UpgradeType);

	FUpgradeDataRow* UpgradeDataRow = DataTableSubSystem->GetUpgradeData(UpgradeType, CurrentGrade);
	int32 Price = bIsMaxGrade ? 0 : DataTableSubSystem->GetUpgradeData(UpgradeType, CurrentGrade + 1)->Price;
	ShopWidget->ShowUpgradeInfos(nullptr, CurrentGrade, bIsMaxGrade, Price, TEXT("임시 텍스트"));
	CurrentSelectedUpgradeType = UpgradeType;

	LOGS(Log, TEXT("UpgradeViewSlotClicked, Index : %d"), ClickedSlotIndex);
}

bool AShop::HasItem(int32 ItemId)
{
	bool bHasItem = (ShopConsumableItemIdList.Contains(ItemId) != INDEX_NONE);
	bHasItem = bHasItem || (ShopEquipmentItemIdList.Contains(ItemId) != INDEX_NONE);

	return bHasItem;
}

bool AShop::IsOpened() const
{
	return bIsOpened;
}

FString AShop::GetInteractionDescription() const
{
	return TEXT("Open Shop!");
}

UADInteractableComponent* AShop::GetInteractableComponent() const
{
	return InteractableComp;
}

bool AShop::IsHoldMode() const
{
	return bIsHold;
}
