
#include "Projectile/PoolableItem.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(ObjectPoolLog);

APoolableItem::APoolableItem()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APoolableItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APoolableItem, ProjectileId);
}

void APoolableItem::Activate()
{
	bIsActive = true;
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);

	OnPoolableItemActivate();
}

void APoolableItem::Deactivate()
{
	bIsActive = false;
	SetOwner(nullptr);
	SetActorLocation(FVector::ZeroVector);
	SetActorRotation(FRotator::ZeroRotator);
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);

	OnPoolableItemDeactivated();
}

void APoolableItem::SetProjectileId(int8 Id)
{
    ProjectileId = Id;
}

void APoolableItem::SetObjectPool(AGenericPool* Pool)
{
    ObjectPool = Pool;
}