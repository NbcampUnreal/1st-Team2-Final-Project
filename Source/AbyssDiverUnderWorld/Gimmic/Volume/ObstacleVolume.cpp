#include "Gimmic/Volume/ObstacleVolume.h"
#include "Character/UnderwaterCharacter.h"
#include "Components/ShapeComponent.h"

AObstacleVolume::AObstacleVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCollisionComponent()->SetCollisionObjectType(ECC_WorldDynamic);
	GetCollisionComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCollisionComponent()->SetCollisionProfileName("BlockAllDynamic");
}

void AObstacleVolume::BeginPlay()
{
	Super::BeginPlay();

	OnActorBeginOverlap.AddDynamic(this, &AObstacleVolume::OnComponentBeginOverlap);
	OnActorEndOverlap.AddDynamic(this, &AObstacleVolume::OnComponentEndOverlap);
	
	GetCollisionComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetCollisionComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetCollisionComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetCollisionComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AObstacleVolume::OnComponentBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
    if (!IsValid(OtherActor)) return;

	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(OtherActor);
	if(!IsValid(Player)) return;
	
	if (OnObstacleComponentBeginOverlapDelegate.IsBound())
	{
		OnObstacleComponentBeginOverlapDelegate.Broadcast(Player);
	}

	Player->SetHideInSeaweed(true);

	LOG(TEXT("Overlapped Begin With Obstacle Volume"));
}

void AObstacleVolume::OnComponentEndOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if (!IsValid(OtherActor)) return;

	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(OtherActor);
	if(!IsValid(Player)) return;
	
	Player->SetHideInSeaweed(false);

	LOG(TEXT("Overlapped End With Obstacle Volume"));
}
