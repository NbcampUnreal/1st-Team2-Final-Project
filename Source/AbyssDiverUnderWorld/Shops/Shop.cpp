#include "Shops/Shop.h"

#include "Character/UnitBase.h"
#include "Character/UnderwaterCharacter.h"
#include "Character/UpgradeComponent.h"

#include "Shops/ShopItemEntryData.h"
#include "Shops/ShopBuyListEntryData.h"

#include "Shops/ShopWidgets/ShopCategoryTabWidget.h"
#include "Shops/ShopWidgets/ShopWidget.h"
#include "Shops/ShopWidgets/ShopElementInfoWidget.h"
#include "Shops/ShopWidgets/ShopItemSlotWidget.h"
#include "Shops/ShopWidgets/ShopBuyListSlotWidget.h"
#include "ShopInteractionComponent.h"

#include "AbyssDiverUnderWorld.h"	
#include "Inventory/ADInventoryComponent.h"
#include "Subsystems/DataTableSubsystem.h"
#include "Subsystems/SoundSubsystem.h"

#include "Framework/ADPlayerState.h"
#include "Framework/ADInGameState.h"

#include "DataRow/UpgradeDataRow.h"
#include "DataRow/FADItemDataRow.h"

#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/PointLightComponent.h"
#include "Engine/TargetPoint.h"

DEFINE_LOG_CATEGORY(ShopLog);

const int8 AShop::MaxItemCount = 99;

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
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	ShopMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShopMesh"));
	SetRootComponent(ShopMeshComponent);
	ShopMeshComponent->SetMobility(EComponentMobility::Static);

	ShopMerchantMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Merchant Mesh"));
	ShopMerchantMeshComponent->SetupAttachment(RootComponent);

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
	//ItemMeshCaptureComp->ShowFlags.PostProcessMaterial = true;
	ItemMeshCaptureComp->ShowFlags.ToneCurve = true;
	//ItemMeshCaptureComp->ShowFlags.Tonemapper = true;

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
	DOREPLIFETIME(AShop, CurrentDoorState);
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

	if (ensureMsgf(OriginPoint, TEXT("Shop의 OriginPoint가 등록되어있지 않습니다. 등록해주세요.")) == false)
	{
		return;
	}

	if (ensureMsgf(DestinationPoint, TEXT("Shop의 DestinationPoint가 등록되어있지 않습니다. 등록해주세요.")) == false)
	{
		return;
	}

	if (HasAuthority() == false)
	{
		PrimaryActorTick.bCanEverTick = false;
		SetActorTickEnabled(false);
	}
}

void AShop::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority())
	{
		int32 ActualLaunchInterval;
		switch (CurrentLaunchType)
		{
		case ELaunchType::First:
			ActualLaunchInterval = LaunchItemIntervalAtFirst;
			break;
		case ELaunchType::InProgress:
			ActualLaunchInterval = LaunchItemInterval;
			break;
		case ELaunchType::Last:
			ActualLaunchInterval = LaunchItemIntervalAtLast;
			break;
		default:
			check(false);
			return;
		}

		if (CurrentDoorState == EDoorState::Opened)
		{
			if (ElapsedTime < ActualLaunchInterval)
			{
				ElapsedTime += DeltaSeconds;
			}
			else
			{
				if (ReadyQueueForLaunchItemById.IsEmpty())
				{
					CurrentDoorState = EDoorState::Closing;
				}
				else
				{
					LaunchItem();
				}
			}
		}
		else if (CurrentDoorState == EDoorState::Opening)
		{
			CurrentDoorRate = FMath::Clamp(CurrentDoorRate + (DeltaSeconds * DoorOpenSpeed), 0, 1);
			RotateDoor(DesiredCloseDegree, DesiredOpenDegree, CurrentDoorRate);
			
			if (CurrentDoorRate == 1)
			{
				CurrentDoorState = EDoorState::Opened;
			}

			if (bIsDoorOpenSoundPlayed == false)
			{
				bIsDoorOpenSoundPlayed = true;
				bIsDoorCloseSoundPlayed = false;

				USoundSubsystem* SoundSubsystem = GetSoundSubsystem();
				if (SoundSubsystem == nullptr)
				{
					return;
				}

				DoorOpenAudioId = SoundSubsystem->Play2D(ESFX::ShopDoorOpen);
				SoundSubsystem->StopAudio(DoorCloseAudioId);
			}
		}
		else if (CurrentDoorState == EDoorState::Closed)
		{
			if (CurrentLaunchType != ELaunchType::First)
			{
				CurrentLaunchType = ELaunchType::First;
			}

			if (ReadyQueueForLaunchItemById.IsEmpty() == false)
			{
				CurrentDoorState = EDoorState::Opening;
			}

			if (bIsDoorCloseSoundPlayed == false)
			{
				bIsDoorOpenSoundPlayed = false;
				bIsDoorCloseSoundPlayed = true;

				USoundSubsystem* SoundSubsystem = GetSoundSubsystem();
				if (SoundSubsystem == nullptr)
				{
					return;
				}

				DoorCloseAudioId = SoundSubsystem->Play2D(ESFX::ShopDoorClose);
				SoundSubsystem->StopAudio(DoorOpenAudioId);
			}
		}
		else /*if (CurrentDoorState == EDoorState::Closing)*/
		{
			CurrentDoorRate = FMath::Clamp(CurrentDoorRate - (DeltaSeconds * DoorCloseSpeed), 0, 1);
			RotateDoor(DesiredCloseDegree, DesiredOpenDegree, CurrentDoorRate);

			if (ReadyQueueForLaunchItemById.IsEmpty() == false)
			{
				CurrentDoorState = EDoorState::Opening;
			}
			else if (CurrentDoorRate == 0)
			{
				CurrentDoorState = EDoorState::Closed;
			}

			if (CurrentLaunchType != ELaunchType::First)
			{
				CurrentLaunchType = ELaunchType::First;
			}
		}
	}
	else
	{
		if (CurrentDoorState == EDoorState::Opened)
		{
			CurrentDoorRate = 1;
			RotateDoor(DesiredCloseDegree, DesiredOpenDegree, CurrentDoorRate);
		}
		else if (CurrentDoorState == EDoorState::Opening)
		{
			CurrentDoorRate = FMath::Clamp(CurrentDoorRate + (DeltaSeconds * DoorOpenSpeed), 0, 1);
			RotateDoor(DesiredCloseDegree, DesiredOpenDegree, CurrentDoorRate);

			if (CurrentDoorRate == 1)
			{
				CurrentDoorState = EDoorState::Opened;
			}

			if (bIsDoorOpenSoundPlayed == false)
			{
				bIsDoorOpenSoundPlayed = true;
				bIsDoorCloseSoundPlayed = false;

				USoundSubsystem* SoundSubsystem = GetSoundSubsystem();
				if (SoundSubsystem == nullptr)
				{
					return;
				}

				DoorOpenAudioId = SoundSubsystem->Play2D(ESFX::ShopDoorOpen);
				SoundSubsystem->StopAudio(DoorCloseAudioId);
			}
		}
		else if (CurrentDoorState == EDoorState::Closed)
		{
			CurrentDoorRate = 0;
			RotateDoor(DesiredCloseDegree, DesiredOpenDegree, CurrentDoorRate);

			if (bIsDoorCloseSoundPlayed == false)
			{
				bIsDoorOpenSoundPlayed = false;
				bIsDoorCloseSoundPlayed = true;

				USoundSubsystem* SoundSubsystem = GetSoundSubsystem();
				if (SoundSubsystem == nullptr)
				{
					return;
				}

				DoorCloseAudioId = SoundSubsystem->Play2D(ESFX::ShopDoorClose);
				SoundSubsystem->StopAudio(DoorOpenAudioId);
			}
		}
		else /*if (CurrentDoorState == EDoorState::Closing)*/
		{
			CurrentDoorRate = FMath::Clamp(CurrentDoorRate - (DeltaSeconds * DoorCloseSpeed), 0, 1);
			RotateDoor(DesiredCloseDegree, DesiredOpenDegree, CurrentDoorRate);

			if (CurrentDoorRate == 0)
			{
				CurrentDoorState = EDoorState::Closed;
			}
		}
	}
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

	USoundSubsystem* SoundSubsystem = GetSoundSubsystem();
	if (SoundSubsystem == nullptr)
	{
		return;
	}

	SoundSubsystem->Play2D(ESFX_UI::ShopOpenClose);
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

				ClearSelectedInfos();
				
			}), RemoveDelay, false);

	USoundSubsystem* SoundSubsystem = GetSoundSubsystem();
	if (SoundSubsystem == nullptr)
	{
		return;
	}

	SoundSubsystem->Play2D(ESFX_UI::ShopOpenClose);
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

	UDataTableSubsystem* DataTableSubsystem = GetDatatableSubsystem();
	if (DataTableSubsystem == nullptr)
	{
		LOGV(Error, TEXT("DataTableSubsystem == nullptr"));
		return EBuyResult::FailedFromOtherReason;
	}

	FFADItemDataRow* ItemDataRow = DataTableSubsystem->GetItemData(ItemId);
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

	FItemData ItemData;
	ItemData.Amount = ItemDataRow->Amount;
	ItemData.CurrentAmmoInMag = ItemDataRow->CurrentAmmoInMag;
	ItemData.ReserveAmmo = ItemDataRow->ReserveAmmo;
	ItemData.Id = ItemDataRow->Id;
	ItemData.ItemType = ItemDataRow->ItemType;
	ItemData.BulletType = ItemDataRow->BulletType;
	ItemData.Mass = ItemDataRow->Weight;
	ItemData.Name = ItemDataRow->Name;
	ItemData.Price = ItemDataRow->Price;
	ItemData.Quantity = Quantity;
	ItemData.Thumbnail = ItemDataRow->Thumbnail;

	if (PS->GetInventory()->AddInventoryItem(ItemData) == false)
	{
		LOGS(Log, TEXT("Buying Item Failed : %s"), *ItemDataRow->Name.ToString());
		return EBuyResult::FailedFromOtherReason;
	}

	GS->SetTotalTeamCredit(TeamCredits - ItemDataRow->Price * Quantity);

	LOGS(Log, TEXT("Buying Item Succeeded : %s"), *ItemDataRow->Name.ToString());
	return EBuyResult::Succeeded;
}

EBuyResult AShop::BuyItems(const TArray<uint8>& ItemIdList, const TArray<int8>& ItemCountList)
{
	if (HasAuthority() == false)
	{
		return EBuyResult::HasNoAuthority;
	}

	int32 TotalItemPrice = CalcTotalItemPrice(ItemIdList, ItemCountList);
	if (TotalItemPrice == INDEX_NONE)
	{
		LOGV(Error, TEXT("Fail to Calculate Total Item Price"));
		return EBuyResult::FailedFromOtherReason;
	}

	AADInGameState* GS = Cast<AADInGameState>(UGameplayStatics::GetGameState(GetWorld()));
	check(GS);

	int32 TeamCredits = GS->GetTotalTeamCredit();
	if (TeamCredits < TotalItemPrice)
	{
		return  EBuyResult::NotEnoughMoney;
	}

	GS->SetTotalTeamCredit(TeamCredits - TotalItemPrice);

	UDataTableSubsystem* DataTableSubsystem = GetDatatableSubsystem();
	if (DataTableSubsystem == nullptr)
	{
		LOGV(Error, TEXT("DataTableSubsystem == nullptr"));
		return EBuyResult::FailedFromOtherReason;
	}

	UWorld* World = GetWorld();
	check(World);

	const int32 ItemIdCount = ItemIdList.Num();
	for (int32 i = 0; i < ItemIdCount; ++i)
	{
		const int32 ItemCountById = ItemCountList[i];
		for (int32 j = 0; j < ItemCountById; ++j)
		{
			ReadyQueueForLaunchItemById.Enqueue(ItemIdList[i]);
		}
	}

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

	UDataTableSubsystem* DataTableSubsystem = GetDatatableSubsystem();
	if (DataTableSubsystem == nullptr)
	{
		LOGV(Error, TEXT("DataTableSubsystem == nullptr"));
		return;
	}

	FFADItemDataRow* ItemDataRow = DataTableSubsystem->GetItemData(ItemId);
	if (ItemDataRow == nullptr)
	{
		LOGV(Error, TEXT("ItemData == nullptr"));
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

	LOGS(Log, TEXT("Enumcount : %d"), EnumCount);
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
			LOGS(Log, TEXT("UpgradeState : UpgradeType(%d) - Grade(%d)"), UpgradeType, Grade);
			continue;
		}
		LOGS(Log, TEXT("UpgradeState(Renewed) : UpgradeType(%d) - Grade(%d)"), UpgradeType, Grade);
		CachedUpgradeGradeMap[i] = Grade;

		FUpgradeDataRow* UpgradeDataRow = DataTableSubsystem->GetUpgradeData(UpgradeType, Grade);

		UShopItemEntryData* UpgradeEntryData = NewObject<UShopItemEntryData>();
		UpgradeEntryData->Init(i, UpgradeDataRow->UpgradeIcon, FString(TEXT("Temp Tooltip")));
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

void AShop::InitShopWidget()
{
	LOGS(Log, TEXT("Init Shop Widget Start"));
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
	ShopWidget->OnShopBuyButtonClickedDelegate.AddUObject(this, &AShop::OnBuyButtonClicked);

	TotalPriceOfBuyList = 0;
	ShopWidget->ChangeTotalPriceText(0);
	
	UShopElementInfoWidget* InfoWidget = ShopWidget->GetInfoWidget();
	check(InfoWidget);

	InfoWidget->Init(ItemMeshComponent);
	InfoWidget->OnAddButtonClickedDelegate.AddUObject(this, &AShop::OnAddButtonClicked);

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
		LOGV(Log, TEXT("Adding ConsumableItem, Id : %d "), Id);
	}

	ShopEquipmentItemIdList.MarkArrayDirty();

	for (const auto& Id : DefaultEquipmentItemIdList)
	{
		AddItemToList(Id, EShopCategoryTab::Equipment);
		LOGV(Log, TEXT("Adding Equipment, Id : %d "), Id);
	}

	ShopWidget->ShowItemViewForTab(EShopCategoryTab::Equipment);
}

void AShop::OnShopItemListChanged(const FShopItemListChangeInfo& Info)
{
	LOGV(Log, TEXT("Begin Change"));

	if (Info.ChangeType >= EShopItemChangeType::Max)
	{
		LOGV(Error, TEXT("Weird Change Type : %d"), Info.ChangeType);
		return;
	}

	if (Info.ShopTab >= EShopCategoryTab::Upgrade)
	{
		LOGV(Error, TEXT("Tab.. Weird.. : %d"), Info.ChangeType);
		return;
	}

	UShopItemEntryData* EntryData = nullptr;
	FFADItemDataRow* ItemDataRow = GetGameInstance()->GetSubsystem<UDataTableSubsystem>()->GetItemData(Info.ItemIdAfter);
	if (ItemDataRow == nullptr)
	{
		LOGV(Error, TEXT("ItemData == nullptr"));
		return;
	}

	switch (Info.ChangeType)
	{
	case EShopItemChangeType::Added:

		EntryData = NewObject<UShopItemEntryData>();
		EntryData->Init(Info.ShopIndex, ItemDataRow->Thumbnail, ItemDataRow->Description); // 임시
		EntryData->OnEntryUpdatedFromDataDelegate.AddUObject(this, &AShop::OnSlotEntryWidgetUpdated);
		
		ShopWidget->AddItem(EntryData, Info.ShopTab);
		LOGV(Log, TEXT("Add Data End, Category : %d"), ItemDataRow->ItemType);
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
	if (SlotEntryWidget->IsA<UShopBuyListSlotWidget>())
	{
		SlotEntryWidget->OnShopItemSlotWidgetClickedDelegate.BindUObject(this, &AShop::OnBuyListEntryClicked);
	}
	else
	{
		SlotEntryWidget->OnShopItemSlotWidgetClickedDelegate.BindUObject(this, &AShop::OnSlotEntryClicked);
		SlotEntryWidget->OnShopItemSlotWidgetDoubleClickedDelegate.BindUObject(this, &AShop::OnSlotEntryDoubleClicked);
	}
}

void AShop::OnSlotEntryClicked(int32 ClickedSlotIndex)
{
	EShopCategoryTab CurrentTab = GetCurrentTab();
	LOGV(Error, TEXT("Clicked Slot Index : %d"), ClickedSlotIndex);
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
		LOGS(Log, TEXT("UpgradeSlot Click!, Index : %d"), ClickedSlotIndex);
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

void AShop::OnSlotEntryDoubleClicked(int32 ClickedSlotIndex)
{
	EShopCategoryTab CurrentTab = GetCurrentTab();
	LOGV(Error, TEXT("Clicked Slot Index : %d"), ClickedSlotIndex);
	if (CurrentTab >= EShopCategoryTab::Max)
	{
		LOGS(Error, TEXT("Weird Tab Type : %d"), CurrentTab);
		return;
	}

	if (CurrentTab == EShopCategoryTab::Upgrade)
	{
		OnBuyButtonClicked();
	}
	else
	{
		OnAddButtonClicked(1);
	}
}

void AShop::OnBuyListEntryClicked(int32 ClickedSlotIndex)
{
	UDataTableSubsystem* DataTableSubsystem = GetDatatableSubsystem();
	if (DataTableSubsystem == nullptr)
	{
		LOGV(Error, TEXT("Fail to get DataTableSubsystem"));
		return;
	}

	uint8 ItemId = SelectedItemIdArray[ClickedSlotIndex];
	FFADItemDataRow* ItemDataRow = DataTableSubsystem->GetItemData(ItemId);
	if (ItemDataRow == nullptr)
	{
		LOGV(Error, TEXT("ItemData == nullptr"));
		return;
	}

	TotalPriceOfBuyList -= ItemDataRow->Price;

	LOGV(Error, TEXT("BuyListEntry Clicked, Index : %d"), ClickedSlotIndex);
	RemoveFromSelecteItemArray(ClickedSlotIndex, 1);
	ShopWidget->RemoveBuyListAt(ClickedSlotIndex, 1);
	ShopWidget->ChangeTotalPriceText(TotalPriceOfBuyList);
}

void AShop::OnAddButtonClicked(int32 Quantity)
{
	EShopCategoryTab CurrentTab = ShopWidget->GetCurrentActivatedTab();
	if (CurrentTab == EShopCategoryTab::Upgrade || CurrentTab == EShopCategoryTab::Max)
	{
		return;
	}

	AADInGameState* GS = CastChecked<AADInGameState>(UGameplayStatics::GetGameState(GetWorld()));
	UDataTableSubsystem* DataTableSubsystem = GetGameInstance()->GetSubsystem<UDataTableSubsystem>();
	FFADItemDataRow* ItemData = DataTableSubsystem->GetItemData(CurrentSelectedItemId);

	UShopBuyListEntryData* BuyListEntryData = ShopWidget->AddToBuyList(CurrentSelectedItemId, Quantity);
	if (BuyListEntryData == nullptr)
	{
		return;
	}

	int32 CurrentQuantity = BuyListEntryData->GetItemCount();

	TotalPriceOfBuyList += ItemData->Price * Quantity;
	ShopWidget->ChangeTotalPriceText(TotalPriceOfBuyList);
	AddToSelectedItemArray(CurrentSelectedItemId, Quantity);

	if (BuyListEntryData->OnEntryUpdatedFromDataDelegate.IsBound())
	{
		return;
	}

	BuyListEntryData->OnEntryUpdatedFromDataDelegate.AddUObject(this, &AShop::OnSlotEntryWidgetUpdated);
}

void AShop::OnBuyButtonClicked()
{
	AADInGameState* GS = CastChecked<AADInGameState>(UGameplayStatics::GetGameState(GetWorld()));
	UDataTableSubsystem* DataTableSubsystem = GetGameInstance()->GetSubsystem<UDataTableSubsystem>();

	int32 TotalTeamCredit = GS->GetTotalTeamCredit();

	EShopCategoryTab CurrentTab = ShopWidget->GetCurrentActivatedTab();

	if (CurrentTab == EShopCategoryTab::Upgrade && CurrentSelectedUpgradeType != EUpgradeType::Max)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		AADPlayerState* PS = PC->GetPlayerState<AADPlayerState>();
		if (PS == nullptr)
		{
			LOGV(Error, TEXT("PS == nullptr"));
			return;
		}

		UUpgradeComponent* UpgradeComp = PS->GetUpgradeComp();
		if (UpgradeComp == nullptr)
		{
			LOGV(Error, TEXT("UpgradeComp == nullptr"));
			return;
		}

		bool bIsMaxGrade = UpgradeComp->IsMaxGrade(CurrentSelectedUpgradeType);
		if (bIsMaxGrade)
		{
			LOGV(Log, TEXT("This Upgrade Type Has Reached to Max Level"));
			return;
		}

		int32 Grade = UpgradeComp->GetCurrentGrade(CurrentSelectedUpgradeType);
		FUpgradeDataRow* UpgradeData = DataTableSubsystem->GetUpgradeData(CurrentSelectedUpgradeType, Grade + 1);

		if (TotalTeamCredit < UpgradeData->Price)
		{
			LOGV(Log, TEXT("업그레이드 돈부족!! 남은 돈 : %d, 필요한 돈 : %d"), TotalTeamCredit, UpgradeData->Price);
			return;
		}

		UpgradeComp->S_RequestUpgrade(CurrentSelectedUpgradeType);
	}
	else
	{
		int32 TotalItemPrice = CalcTotalItemPrice(SelectedItemIdArray, SelectedItemCountArray);
		if (TotalItemPrice == INDEX_NONE)
		{
			LOGV(Error, TEXT("Fail to Calculate Total Item Price"));
			return;
		}

		if (TotalTeamCredit < TotalItemPrice)
		{
			LOGV(Log, TEXT("Not Enough Money, Needed : %d, Held : %d"), TotalItemPrice, TotalTeamCredit);
			return;
		}

		AUnderwaterCharacter* BuyingCharacter = Cast<AUnderwaterCharacter>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetPawn());
		if (BuyingCharacter == nullptr)
		{
			LOGV(Warning, TEXT("BuyingCharacter == nullptr"));
			return;
		}

		UShopInteractionComponent* ShopInteractionComp = BuyingCharacter->GetShopInteractionComponent();
		if (ShopInteractionComp == nullptr)
		{
			LOGV(Warning, TEXT("ShopInteractionComp == nullptr"));
			return;
		}

		ShopInteractionComp->S_RequestBuyItems(SelectedItemIdArray, SelectedItemCountArray);
		ClearSelectedInfos();
	}
}

void AShop::OnCloseButtonClicked()
{
	AUnderwaterCharacter* Requester = Cast<AUnderwaterCharacter>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetPawn());
	if (Requester == nullptr)
	{
		LOGV(Error, TEXT("Requester == nullptr"));
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
		LOGV(Error, TEXT("PS == nullptr"));
		return;
	}

	UUpgradeComponent* UpgradeComp = PS->GetUpgradeComp();
	if (UpgradeComp == nullptr)
	{
		LOGV(Error, TEXT("UpgradeComp == nullptr"));
		return;
	}

	EUpgradeType UpgradeType = EUpgradeType(ClickedSlotIndex);
	
	uint8 CurrentGrade = UpgradeComp->GetCurrentGrade(UpgradeType);
	if (CurrentGrade == 0)
	{
		LOGV(Log, TEXT("Weird Upgrade Type Detected : %d"), UpgradeType);
		return;
	}

	UDataTableSubsystem* DataTableSubSystem = GetGameInstance()->GetSubsystem<UDataTableSubsystem>();
	if (DataTableSubSystem == nullptr)
	{
		LOGV(Error, TEXT("DataTableSubSystem == nullptr"));
		return;
	}

	bool bIsMaxGrade = UpgradeComp->IsMaxGrade(UpgradeType);

	FUpgradeDataRow* UpgradeDataRow = DataTableSubSystem->GetUpgradeData(UpgradeType, CurrentGrade);
	int32 Price = bIsMaxGrade ? 0 : DataTableSubSystem->GetUpgradeData(UpgradeType, CurrentGrade + 1)->Price;
	ShopWidget->ShowUpgradeInfos(UpgradeType, CurrentGrade, bIsMaxGrade);
	CurrentSelectedUpgradeType = UpgradeType;

	LOGV(Log, TEXT("UpgradeViewSlotClicked, Index : %d"), ClickedSlotIndex);
}

int8 AShop::IsSelectedItem(uint8 ItemId) const
{
	int32 SelectedItemCount = SelectedItemIdArray.Num();
	for (int32 i = 0; i < SelectedItemCount; ++i)
	{
		if (ItemId == SelectedItemIdArray[i])
		{
			return i;
		}
	}

	return INDEX_NONE;
}

void AShop::AddToSelectedItemArray(uint8 ItemId, int8 Amount)
{
	if (Amount > MaxItemCount || Amount < 1)
	{
		return;
	}

	int8 SelectedItemIndex = IsSelectedItem(ItemId);
	if (SelectedItemIndex == INDEX_NONE)
	{
		SelectedItemIdArray.Add(ItemId);
		SelectedItemCountArray.Add(Amount);
	}
	else
	{
		int8& SelectedItemCount = SelectedItemCountArray[SelectedItemIndex];
		int8 ActualAddedAmount = FMath::Clamp(SelectedItemCount + Amount, 0, MaxItemCount);
		SelectedItemCount = ActualAddedAmount;
	}
}

void AShop::RemoveFromSelecteItemArray(uint8 ItemId, int8 Amount)
{
	if (Amount < 1)
	{
		return;
	}

	int8 SelectedItemIndex = IsSelectedItem(ItemId);
	if (SelectedItemIndex == INDEX_NONE)
	{
		return;
	}

	RemoveFromSelecteItemArray(SelectedItemIndex, Amount);
}

void AShop::RemoveFromSelecteItemArray(int32 BuyListSlotIndex, int8 Amount)
{
	int8& SelectedItemCount = SelectedItemCountArray[BuyListSlotIndex];
	if (SelectedItemCount <= Amount)
	{
		SelectedItemCountArray.RemoveAt(BuyListSlotIndex);
		SelectedItemIdArray.RemoveAt(BuyListSlotIndex);
	}
	else 
	{
		SelectedItemCount -= Amount;
	}
}

UDataTableSubsystem* AShop::GetDatatableSubsystem()
{
	UGameInstance* GI = GetGameInstance();
	if (GI == nullptr)
	{
		LOGV(Error, TEXT("GI == nullptr"));
		return nullptr;
	}

	UDataTableSubsystem* DataTableSubsystem = GI->GetSubsystem<UDataTableSubsystem>();
	if (DataTableSubsystem == nullptr)
	{
		LOGV(Error, TEXT("DataTableSubsystem == nullptr"));
	}

	return DataTableSubsystem;
}

int32 AShop::CalcTotalItemPrice(const TArray<uint8>& ItemIdList, const TArray<int8>& ItemCountList)
{
	if (ItemIdList.Num() != ItemCountList.Num())
	{
		LOGV(Error, TEXT("Number of ItemIdList not equal to Number of ItemCountList"));
		return INDEX_NONE;
	}

	UDataTableSubsystem* DataTableSubsystem = GetDatatableSubsystem();
	if (DataTableSubsystem == nullptr)
	{
		LOGV(Error, TEXT("DataTableSubsystem == nullptr"));
		return INDEX_NONE;
	}

	UWorld* World = GetWorld();
	check(World);

	int32 TotalItemPrice = 0;

	const int32 ItemIdCount = ItemIdList.Num();
	for (int32 i = 0; i < ItemIdCount; ++i)
	{
		FFADItemDataRow* ItemDataRow = DataTableSubsystem->GetItemData(ItemIdList[i]);
		if (ItemDataRow == nullptr)
		{
			LOGV(Error, TEXT("ItemData == nullptr"));
			return INDEX_NONE;
		}

		TotalItemPrice += ItemDataRow->Price * ItemCountList[i];
	}

	return TotalItemPrice;
}

void AShop::LaunchItem()
{
	ElapsedTime = 0.0f;
	CurrentLaunchType = ELaunchType::InProgress;

	if (ReadyQueueForLaunchItemById.IsEmpty())
	{
		return;
	}

	uint8 ItemId = INDEX_NONE;

	if (ReadyQueueForLaunchItemById.Dequeue(ItemId) == false)
	{
		LOGV(Warning, TEXT("Falil to Get Spawned Item From Ready Queue"));
		return;
	}

	if (ItemId == INDEX_NONE)
	{
		LOGV(Error, TEXT("Fail to get Item Id"));
		return;
	}

	UDataTableSubsystem* DataTableSubsystem = GetDatatableSubsystem();
	if (DataTableSubsystem == nullptr)
	{
		LOGV(Error, TEXT("DataTableSubsystem == nullptr"));
		return;
	}

	FFADItemDataRow* ItemDataRow = DataTableSubsystem->GetItemData(ItemId);
	if (ItemDataRow == nullptr)
	{
		LOGV(Error, TEXT("ItemData == nullptr"));
		return;
	}

	FItemData ItemData
	(
		ItemDataRow->Name,
		ItemDataRow->Id,
		1,
		0,
		ItemDataRow->Amount,
		ItemDataRow->CurrentAmmoInMag,
		ItemDataRow->ReserveAmmo,
		ItemDataRow->Weight,
		ItemDataRow->Price,
		ItemDataRow->ItemType,
		ItemDataRow->BulletType,
		ItemDataRow->Thumbnail
	);

	UWorld* World = GetWorld();
	check(World);

	AADUseItem* SpawnedItem = World->SpawnActor<AADUseItem>(AADUseItem::StaticClass(), OriginPoint->GetActorTransform());
	if (SpawnedItem == nullptr || IsValid(SpawnedItem) == false || SpawnedItem->IsPendingKillPending())
	{
		LOGV(Error, TEXT("Not Valid Spawned Item"));
		return;
	}

	SpawnedItem->SetItemInfo(ItemData, false, EEnvironmentState::Ground);

	UPrimitiveComponent* ItemRoot = Cast<UPrimitiveComponent>(SpawnedItem->GetRootComponent());
	if (ItemRoot == nullptr)
	{
		LOGV(Error, TEXT("ItemRoot == nullptr"));
		return;
	}

	SpawnedItem->M_SetItemVisible(true);

	FVector Destination = DestinationPoint->GetActorLocation();
	Destination.X += FMath::RandRange(-ErrorOfLaunchDirection, ErrorOfLaunchDirection);
	Destination.Y += FMath::RandRange(-ErrorOfLaunchDirection, ErrorOfLaunchDirection);
	Destination.Z += FMath::RandRange(-ErrorOfLaunchDirection, ErrorOfLaunchDirection);

	FVector LaunchDirection = Destination - OriginPoint->GetActorLocation();
	LaunchDirection.Normalize();

	float ItemMeshMass = SpawnedItem->GetMeshMass();
	if (ItemMeshMass == 0.0f)
	{
		LOGV(Error, TEXT("Invalid Mesh"));
		return;
	}

	float ActualForce = ForceAmount * ItemMeshMass;
	ItemRoot->AddImpulse(LaunchDirection * ActualForce);
	LOGV(Warning, TEXT("Launch"));
	
	if (ReadyQueueForLaunchItemById.IsEmpty())
	{
		CurrentLaunchType = ELaunchType::Last;
	}
}

void AShop::RotateDoor(float DegreeFrom, float DegreeTo, float Rate)
{
	if (DoorActor == nullptr)
	{
		return;
	}

	float CurrentDegree = FMath::Lerp(DegreeFrom, DegreeTo, Rate);

	FRotator CurrentDoorRotation = DoorActor->GetActorRotation();
	CurrentDoorRotation.Yaw = CurrentDegree;
	DoorActor->SetActorRotation(CurrentDoorRotation);
}

void AShop::ClearSelectedInfos()
{
	SelectedItemCountArray.Reset();
	SelectedItemIdArray.Reset();
	ShopWidget->RemoveBuyListAll();
	TotalPriceOfBuyList = 0;
	ShopWidget->ChangeTotalPriceText(TotalPriceOfBuyList);
}

EShopCategoryTab AShop::GetCurrentTab()
{
	return (IsValid(ShopWidget)) ? ShopWidget->GetCurrentActivatedTab() : EShopCategoryTab::Max;
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

USoundSubsystem* AShop::GetSoundSubsystem()
{
	UWorld* World = GetWorld();

	if (IsValid(World) == false || World->bIsTearingDown)
	{
		return nullptr;
	}

	UGameInstance* GI = UGameplayStatics::GetGameInstance(World);
	if (GI == nullptr)
	{
		return nullptr;
	}

	USoundSubsystem* SoundSubsystem = GI->GetSubsystem<USoundSubsystem>();
	if (SoundSubsystem == nullptr)
	{
		return nullptr;
	}

	return SoundSubsystem;
}

UADInteractableComponent* AShop::GetInteractableComponent() const
{
	return InteractableComp;
}

bool AShop::IsHoldMode() const
{
	return bIsHold;
}
