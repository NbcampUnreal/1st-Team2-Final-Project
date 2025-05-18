// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Monster.h"
#include "Components/SplineComponent.h"
#include "Monster/SplinePathActor.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbyssDiverUnderWorld.h"

AMonster::AMonster()
{
	bUseControllerRotationYaw = false;
}

void AMonster::BeginPlay()
{
	Super::BeginPlay();

	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->bOrientRotationToMovement = true;
		MovementComp->RotationRate = FRotator(0.0f, 180.0f, 0.0f);
	}
}

FVector AMonster::GetPatrolLocation(int32 Index) const
{
	if (AssignedSplineActor)
	{
		USplineComponent* PatrolSpline = AssignedSplineActor->GetSplineComponent();
		if (!PatrolSpline || PatrolSpline->GetNumberOfSplinePoints() == 0) return GetActorLocation();

		// Get SplinePoint to World Location
		FVector Location = PatrolSpline->GetLocationAtSplinePoint(Index, ESplineCoordinateSpace::World);
		return Location;
	}
	else
	{
		LOG(TEXT("SplineActor is not Assigned."));
		return FVector::ZeroVector;
	}
}

int32 AMonster::GetNextPatrolIndex(int32 CurrentIndex) const
{
	if (AssignedSplineActor)
	{
		USplineComponent* PatrolSpline = AssignedSplineActor->GetSplineComponent();
		CurrentIndex = (CurrentIndex + 1) % PatrolSpline->GetNumberOfSplinePoints(); // Cycle
		LOG(TEXT("PatrolIndex increment : %d"), CurrentIndex);

		return CurrentIndex;
	}
	else
	{
		LOG(TEXT("SplineActor is not Assigned."));
		return -1;
	}
}

void AMonster::M_PlayAttackMontage_Implementation()
{
	if (AttackMontage)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(AttackMontage);
		}
	}
}


