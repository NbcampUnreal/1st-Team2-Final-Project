#include "Interactable/Item/ADItemBase.h"
#include "AbyssDiverUnderWorld.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Interactable/Item/Component/ADInteractableComponent.h"
#include "Inventory/ADInventoryComponent.h"
#include "Framework/ADPlayerState.h"

// Sets default values
AADItemBase::AADItemBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;


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

	LOG(TEXT("Add to Inventory"));
	// TODO 인벤토리 추가 로직과 획득 효과 추가
	if (UADInventoryComponent* Inventory = PS->GetInventory())
	{
		LOG(TEXT("Find Inventory"));
		Inventory->AddInventoryItem(ItemData);
	}

	Destroy();
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


