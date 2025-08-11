#include "Interactable/Item/ADItemBase.h"

#include "AbyssDiverUnderWorld.h"

#include "Interactable/Item/Component/ADInteractableComponent.h"
#include "Inventory/ADInventoryComponent.h"
#include "Character/UnderwaterCharacter.h"

#include "Framework/ADPlayerState.h"
#include "Framework/ADGameInstance.h"

#include "Subsystems/SoundSubsystem.h"
#include "Subsystems/Localizations/LocalizationSubsystem.h"

#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"

DEFINE_LOG_CATEGORY(ItemLog);

// Sets default values
AADItemBase::AADItemBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bIsHold = false;

	// 발사체 모션 세팅
	DropMovement = CreateDefaultSubobject<UProjectileMovementComponent>("DropMovement");
	//DropMovement->SetUpdatedComponent(MeshComponent);
	DropMovement->bAutoActivate = false;
	DropMovement->ProjectileGravityScale = WaterGravityScale;
	DropMovement->InitialSpeed = 0.f;
	DropMovement->MaxSpeed = 0.f;
	DropMovement->bRotationFollowsVelocity = false;
	DropMovement->bShouldBounce = true;            // 살짝 튕기게
	DropMovement->Bounciness = 0.3f;
	DropMovement->SetIsReplicated(true);
	SetReplicateMovement(true);

	// InteractableComponent 생성
	InteractableComp = CreateDefaultSubobject<UADInteractableComponent>(TEXT("InteractableComp"));
}

void AADItemBase::BeginPlay()
{
	Super::BeginPlay();

	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		SoundSubsystem = GI->GetSubsystem<USoundSubsystem>();
	}
}

void AADItemBase::Interact_Implementation(AActor* InstigatorActor)
{
	if (HasAuthority())
	{
		HandlePickup(Cast<APawn>(InstigatorActor));
	}
}

void AADItemBase::HandlePickup(APawn* InstigatorPawn)
{
	if (!HasAuthority() || !InstigatorPawn) return;
	
	APlayerController* PC = Cast<APlayerController>(InstigatorPawn->GetController());
	AADPlayerState* PS = Cast<AADPlayerState>(PC->PlayerState);
	AUnderwaterCharacter* Diver = Cast<AUnderwaterCharacter>(InstigatorPawn);
	if (!PC || !PS || !Diver) return;

	//-------------------------------- Bullet 아이템 처리
	

	if (ItemData.ItemType == EItemType::Bullet)
	{
		int32 AmmoPerPickup = ItemData.Amount;
		if (UADInventoryComponent* Inv = PS->GetInventory())
		{
			const bool bIsMatchedEquipInInventory = Inv->TryGiveAmmoToEquipment(ItemData.BulletType, AmmoPerPickup);
			if (bIsMatchedEquipInInventory)
			{
				LOGI(Log, TEXT("Matched Bullet!! Add Ammo!!"));
				M_PlayPickupSound();
				Destroy();
			}
			LOGI(Log, TEXT("No Matched Bullet!! Buy Matched Gun"));
			return;
		}
		return;
	}
	LOGI(Log, TEXT("Add to Inventory"));
	//-------------------------------- 일반 아이템 처리
	if (UADInventoryComponent* Inventory = PS->GetInventory())
	{
		LOGI(Log, TEXT("Find Inventory"));
		if (Inventory->AddInventoryItem(ItemData))
		{
			M_PlayPickupSound();
			Destroy();
		}
	}
}

void AADItemBase::M_PlayPickupSound_Implementation()
{
	GetSoundSubsystem()->PlayAt(ESFX::Pickup, GetActorLocation(), 2.0f);
}

void AADItemBase::OnRep_ItemData()
{
	// TODO UI 업데이트
}

void AADItemBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AADItemBase, ItemData);
}

void AADItemBase::SetItemMass(int32 InMass)
{
	ItemData.Mass = InMass;
	ItemData.Quantity = 1;
}

void AADItemBase::SetPrice(int32 InPrice)
{
	ItemData.Price = InPrice;
}

UADInteractableComponent* AADItemBase::GetInteractableComponent() const
{
	return InteractableComp;
}

bool AADItemBase::IsHoldMode() const
{
	return bIsHold;
}

FString AADItemBase::GetInteractionDescription() const
{
	ULocalizationSubsystem* LocalizationSubsystem = GetGameInstance()->GetSubsystem<ULocalizationSubsystem>();
	if (IsValid(LocalizationSubsystem) == false)
	{
		LOGV(Error, TEXT("Cant Get LocalizationSubsystem"));
		return "";
	}

	return LocalizationSubsystem->GetLocalizedText(ST_InteractionDescription::TableKey, ST_InteractionDescription::ItemBase_Pickup).ToString();
}

USoundSubsystem* AADItemBase::GetSoundSubsystem()
{
	if (SoundSubsystem)
	{
		return SoundSubsystem;
	}

	if (UADGameInstance* GI = Cast<UADGameInstance>(GetWorld()->GetGameInstance()))
	{
		SoundSubsystem = GI->GetSubsystem<USoundSubsystem>();
		return SoundSubsystem;
	}
	return nullptr;
}
