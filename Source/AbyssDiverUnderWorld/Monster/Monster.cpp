// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Monster.h"
#include "Components/SplineComponent.h"

AMonster::AMonster()
{
	PatrolSpline = CreateDefaultSubobject<USplineComponent>(TEXT("PatrolSpline"));
	PatrolSpline->SetupAttachment(RootComponent);
}

void AMonster::BeginPlay()
{
	Super::BeginPlay();
}

FVector AMonster::GetPatrolLocation(int32 Index) const
{
	if (!PatrolSpline || PatrolSpline->GetNumberOfSplinePoints() == 0) return GetActorLocation();

	// Get SplinePoint to World Location
	FVector Location = PatrolSpline->GetLocationAtSplinePoint(Index, ESplineCoordinateSpace::World);
	return Location;
}

int32 AMonster::GetNextPatrolIndex(int32 CurrentIndex) const
{
	return (CurrentIndex + 1) % PatrolSpline->GetNumberOfSplinePoints(); // Cycle
}

USplineComponent* AMonster::GetSplineComp() const
{
	return PatrolSpline.Get();
}

