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

FVector AMonster::FindNextPatrolLocation(int32& InOutIndex) const
{
	if (!PatrolSpline || PatrolSpline->GetNumberOfSplinePoints() == 0) return GetActorLocation();

	// Get SplinePoint to World Location
	FVector Location = PatrolSpline->GetLocationAtSplinePoint(InOutIndex, ESplineCoordinateSpace::World); 
	InOutIndex = (InOutIndex + 1) % PatrolSpline->GetNumberOfSplinePoints(); // Cycle

	return Location;
}

USplineComponent* AMonster::GetSplineComp() const
{
	return PatrolSpline.Get();
}

