#include "Interactable/Item/ADItemBase.h"
#include "AbyssDiverUnderWorld.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Interactable/Item/Component/ADInteractableComponent.h"
#include "Inventory/ADInventoryComponent.h"
#include "Framework/ADPlayerState.h"
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

	LOGI(Log, TEXT("Add to Inventory"));
	// TODO 인벤토리 추가 로직과 획득 효과 추가
	if (UADInventoryComponent* Inventory = PS->GetInventory())
	{
		LOGI(Log, TEXT("Find Inventory"));
		if (Inventory->AddInventoryItem(ItemData))
		{
			Destroy();
		}
	}
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
	return TEXT("Pick up!");
}
