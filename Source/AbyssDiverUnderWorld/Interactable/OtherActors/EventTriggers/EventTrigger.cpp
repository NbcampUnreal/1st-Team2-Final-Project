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

#if WITH_EDITOR

	// 게임 중이 아닌 경우 리턴(블루프린트 상일 경우)
	// PostInitializeComponents는 블루프린트에서도 발동함
	UWorld* World = GetWorld();
	if (World == nullptr || World->IsGameWorld() == false)
	{
		return;
	}

#endif

	// 호스트만 사용
	if (HasAuthority() == false)
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


