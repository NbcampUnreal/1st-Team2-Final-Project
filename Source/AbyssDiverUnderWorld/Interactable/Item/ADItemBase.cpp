#include "Interactable/Item/ADItemBase.h"
#include "AbyssDiverUnderWorld.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Interactable/Item/Component/ADInteractableComponent.h"
#include "Inventory/ADInventoryComponent.h"

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

void AADItemBase::Interact(AActor* InstigatorActor)
{
	if (HasAuthority())
	{
		HandlePickup(Cast<APawn>(InstigatorActor));
	}
}

void AADItemBase::HandlePickup(APawn* InstigatorPawn)
{
	if (!HasAuthority() || !InstigatorPawn) return;

	LOG(TEXT("Add to Inventory"));
	// TODO 인벤토리 추가 로직과 획득 효과 추가
	if (UADInventoryComponent* Inventory = InstigatorPawn->FindComponentByClass<UADInventoryComponent>())
	{
		Inventory->AddInventoryItem(ItemData, 1);
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
}

void AADItemBase::SetPrice(int32 InPrice)
{
	ItemData.Price = InPrice;
}


