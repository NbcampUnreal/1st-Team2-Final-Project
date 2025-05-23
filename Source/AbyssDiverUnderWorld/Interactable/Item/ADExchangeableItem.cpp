#include "Interactable/Item/ADExchangeableItem.h"
#include "Net/UnrealNetwork.h"
#include "AbyssDiverUnderWorld.h"
#include "GameFramework/ProjectileMovementComponent.h"

AADExchangeableItem::AADExchangeableItem()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("MeshComponent");
	RootComponent = MeshComponent;
	MeshComponent->SetMobility(EComponentMobility::Movable); 
	MeshComponent->SetIsReplicated(true);
	bReplicates = true;
}
void AADExchangeableItem::BeginPlay()
{
	Super::BeginPlay();
	CalculateTotalPrice();
}

void AADExchangeableItem::OnRep_TotalPrice()
{
	// TODO 클라이언트 UI 갱신
}

void AADExchangeableItem::Interact_Implementation(AActor* InstigatorActor)
{
	if (!HasAuthority()) return;
	LOGI(Log, TEXT("Mineral's Mass : %d"), ItemData.Mass);
	ItemData.Price = TotalPrice;
	LOGI(Log, TEXT("Mineral's Price : %d"), ItemData.Price);

	HandlePickup(Cast<APawn>(InstigatorActor));
}

void AADExchangeableItem::CalculateTotalPrice()
{
	TotalPrice = ItemData.Mass * ValuePerUnit;
}

void AADExchangeableItem::HandlePickup(APawn* InstigatgorPawn)
{
	Super::HandlePickup(InstigatgorPawn);

	// TODO 개별 획득 사운드가 필요하다면 추가하는 것이 좋아보임
}

void AADExchangeableItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AADExchangeableItem, TotalPrice);
}
