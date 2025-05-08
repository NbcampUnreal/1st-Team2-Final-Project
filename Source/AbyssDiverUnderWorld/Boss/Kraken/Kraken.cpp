#include "Boss/Kraken/Kraken.h"

#include "AbyssDiverUnderWorld.h"

AKraken::AKraken()
{
}

void AKraken::BeginPlay()
{
	Super::BeginPlay();
}

void AKraken::Move()
{
	Super::Move();
	
	if (CurrentPatrolPointIndex >= PatrolPoints.Num())
	{
		CurrentPatrolPointIndex = 0;
	}
	
	const FVector TargetLocation = PatrolPoints[CurrentPatrolPointIndex]->GetActorLocation();
	FlyingMoveToTarget(TargetLocation);

	++CurrentPatrolPointIndex;
}

void AKraken::MoveStop()
{
	Super::MoveStop();
	FlyingMoveToTargetStop();
}

void AKraken::MoveToTarget()
{
	Super::MoveToTarget();
	if (!IsValid(TargetPlayer)) return;
	FlyingMoveToTarget(TargetPlayer->GetActorLocation());
	LOG(TEXT("%s"), *TargetPlayer->GetActorLocation().ToString())
}

void AKraken::MoveToLastDetectedLocation()
{
	Super::MoveToLastDetectedLocation();
	FlyingMoveToTarget(LastDetectedLocation);
}
