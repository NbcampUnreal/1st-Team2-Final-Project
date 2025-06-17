#include "Interactable/Item/ADExchangeableItem.h"
#include "Net/UnrealNetwork.h"
#include "AbyssDiverUnderWorld.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/PointLightComponent.h"

AADExchangeableItem::AADExchangeableItem()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("MeshComponent");
	RootComponent = MeshComponent;
	MeshComponent->SetMobility(EComponentMobility::Movable); 
	MeshComponent->SetIsReplicated(true);
	MeshComponent->SetCollisionProfileName(TEXT("BlockAllDynamicAndInteraction"));
	bReplicates = true;
}
void AADExchangeableItem::BeginPlay()
{
	Super::BeginPlay();
	CalculateTotalPrice();

	DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(0);
	if (DynamicMaterial)
	{
		DynamicMaterial->SetScalarParameterValue(TEXT("GlowPower"), MinGlow);
	}

	GetWorld()->GetTimerManager().SetTimer(
		PulseTimerHandle,
		this,
		&AADExchangeableItem::UpdateGlow,
		0.03f,
		true
	);
}

void AADExchangeableItem::OnRep_TotalPrice()
{
	// TODO 클라이언트 UI 갱신
}

void AADExchangeableItem::Interact_Implementation(AActor* InstigatorActor)
{
	if (!HasAuthority()) return;

	HandlePickup(Cast<APawn>(InstigatorActor));
}

void AADExchangeableItem::CalculateTotalPrice()
{
	TotalPrice = ItemData.Mass * ValuePerUnit;
	LOGI(Log, TEXT("Mineral's Mass : %d"), ItemData.Mass);
	ItemData.Price = TotalPrice;
	LOGI(Log, TEXT("Mineral's Price : %d"), ItemData.Price);
}

void AADExchangeableItem::HandlePickup(APawn* InstigatorPawn)
{
	Super::HandlePickup(InstigatorPawn);

	// 델리게이트
	const int32 PickedItemID = ItemData.Id;
	OnItemPicked.Broadcast(PickedItemID, InstigatorPawn);
}

void AADExchangeableItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AADExchangeableItem, TotalPrice);
}

void AADExchangeableItem::UpdateGlow()
{
	if (!DynamicMaterial) return;

	const float TimeSec = GetWorld()->GetTimeSeconds();
	const float Raw = FMath::Sin(TimeSec * PulseFrequency * 2 * PI);

	const float Normalized = (Raw + 1.f) * 0.5f;

	const float GlowValue = FMath::Lerp(MinGlow, MaxGlow, Normalized);

	DynamicMaterial->SetScalarParameterValue(TEXT("GlowPower"), GlowValue);
}
