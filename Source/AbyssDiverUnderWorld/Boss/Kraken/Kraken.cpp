#include "Boss/Kraken/Kraken.h"

AKraken::AKraken()
{
}

void AKraken::BeginPlay()
{
	Super::BeginPlay();

	Move();
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

	SetMoveStopTimer();
}

void AKraken::MoveStop()
{
	Super::MoveStop();

	FlyingMoveToTargetStop();
	
	SetMoveTimer();
}
