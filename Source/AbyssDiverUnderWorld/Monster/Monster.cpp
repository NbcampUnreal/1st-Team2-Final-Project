// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Monster.h"
#include "Components/SplineComponent.h"
#include "Monster/SplinePathActor.h"
#include "Character/StatComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbyssDiverUnderWorld.h"

const FName AMonster::MonsterStateKey = "MonsterState";

AMonster::AMonster()
{
	bUseControllerRotationYaw = false;
}

void AMonster::BeginPlay()
{
	Super::BeginPlay();

	AnimInstance = GetMesh()->GetAnimInstance();
	AIController = Cast<AMonsterAIController>(GetController());

	if (IsValid(AIController))
	{
		BlackboardComponent = AIController->GetBlackboardComponent();
	}

	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->bOrientRotationToMovement = true;
		MovementComp->RotationRate = FRotator(0.0f, 180.0f, 0.0f);
	}
}

void AMonster::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMonster, MonsterState);
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

void AMonster::M_PlayAttackMontage_Implementation(UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	if (AttackMontage)
	{
		PlayAnimMontage(AttackMontage);
	}
}

float AMonster::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float Damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (IsValid(StatComponent))
	{
		if (StatComponent->GetCurrentHealth() <= 0)
		{
			OnDeath();
		}
	}
	return Damage;
}

void AMonster::OnDeath()
{
	AIController->StopMovement();
	AnimInstance->StopAllMontages(0.5f);

	SetMonsterState(EMonsterState::Death);

	AIController->UnPossess();
}

void AMonster::SetMonsterState(EMonsterState State)
{
	if (!HasAuthority()) return;

	MonsterState = State;

	BlackboardComponent ->SetValueAsEnum(MonsterStateKey, static_cast<uint8>(MonsterState));
}



