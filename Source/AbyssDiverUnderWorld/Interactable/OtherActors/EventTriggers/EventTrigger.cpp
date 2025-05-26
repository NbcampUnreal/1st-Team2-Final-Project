#include "Interactable/OtherActors/EventTriggers/EventTrigger.h"

#include "AbyssDiverUnderWorld.h"

#include "Components/BoxComponent.h"
#include "Components/BillboardComponent.h"

AEventTrigger::AEventTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);

	BillboardSprite = CreateDefaultSubobject<UBillboardComponent>(TEXT("Editor BillboardSprite"));
	BillboardSprite->SetupAttachment(RootComponent);
}

void AEventTrigger::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// ���� ���� �ƴ� ��� ����(�������Ʈ ���� ���)
	// PostInitializeComponents�� �������Ʈ������ �ߵ���
	UWorld* World = GetWorld();
	if (World == nullptr || World->IsGameWorld() == false)
	{
		return;
	}

	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AEventTrigger::OnBeginOverlap);
}

void AEventTrigger::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	for (const auto& TriggerableActor : TriggerableActors)
	{
		ITriggerable::Execute_TriggerEvent(TriggerableActor, OtherActor);
	}
}


