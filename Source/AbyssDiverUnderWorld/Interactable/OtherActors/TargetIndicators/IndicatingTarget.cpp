#include "Interactable/OtherActors/TargetIndicators/IndicatingTarget.h"

#include "Components/BoxComponent.h"
#include "Components/BillboardComponent.h"

AIndicatingTarget::AIndicatingTarget()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);

	BillboardSprite = CreateDefaultSubobject<UBillboardComponent>(TEXT("Editor BillboardSprite"));
	BillboardSprite->SetupAttachment(RootComponent);

	if (CollisionBox->OnComponentBeginOverlap.IsBound())
	{
		return;
	}

	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AIndicatingTarget::OnBeginOverlap);

	bReplicates = false;
}

void AIndicatingTarget::BeginPlay()
{
	Super::BeginPlay();

	if (SwitchActor == nullptr)
	{
		return;
	}

	if (SwitchActor->OnDestroyed.IsAlreadyBound(this, &AIndicatingTarget::OnSwitchActorDestroyed))
	{
		return;
	}

	SwitchActor->OnDestroyed.AddDynamic(this, &AIndicatingTarget::OnSwitchActorDestroyed);
}

bool AIndicatingTarget::IsActivateConditionMet()
{
	return (SwitchActor == nullptr || IsValid(SwitchActor) == false || SwitchActor->IsPendingKillPending());
}

void AIndicatingTarget::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APawn* Player = Cast<APawn>(OtherActor);
	if (Player == nullptr)
	{
		return;
	}

	if (Player->IsLocallyControlled() == false)
	{
		return;
	}

	OnIndicatingTargetBeginOverlapDelegate.Broadcast(TargetOrder);
}

void AIndicatingTarget::OnSwitchActorDestroyed(AActor* DestroyedActor)
{
	OnSwitchActorDestroyedDelegate.Broadcast(TargetOrder);
}

int32 AIndicatingTarget::GetTargetOrder() const
{
	return TargetOrder;
}

UTexture2D* AIndicatingTarget::GetTargetIcon() const
{
	return TargetIcon;
}