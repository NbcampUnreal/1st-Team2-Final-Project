// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/SplinePathActor.h"
#include "Components/SplineComponent.h"

// Sets default values
ASplinePathActor::ASplinePathActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("PatrolSpline"));
	RootComponent = SplineComponent;
}




