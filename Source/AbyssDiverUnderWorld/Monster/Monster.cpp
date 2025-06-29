// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Monster.h"
#include "Components/SplineComponent.h"
#include "Monster/SplinePathActor.h"
#include "Character/StatComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbyssDiverUnderWorld.h"
#include "Interactable/OtherActors/Radars/RadarReturnComponent.h"

const FName AMonster::MonsterStateKey = "MonsterState";
const FName AMonster::InvestigateLocationKey = "InvestigateLocation";
const FName AMonster::TargetActorKey = "TargetActor";

AMonster::AMonster()
{
	// Initialize
	AssignedSplineActor = nullptr;
	BlackboardComponent = nullptr;
	AIController = nullptr;
	AnimInstance = nullptr;
	TargetActor = nullptr;
	ChaseTriggerTime = 3.0f;
	ChaseSpeed = 400.0f;
	PatrolSpeed = 200.0f;
	InvestigateSpeed = 300.0f;
	bIsChasing = false;

	bUseControllerRotationYaw = false;
	bReplicates = true;
	SetReplicatingMovement(true);

	RadarReturnComponent->FactionTags.Init(TEXT("Hostile"), 1);
}

void AMonster::BeginPlay()
{
	Super::BeginPlay();

	AnimInstance = GetMesh()->GetAnimInstance();
	AIController = Cast<AMonsterAIController>(GetController());

	if (IsValid(AIController))
	{
		BlackboardComponent = AIController->GetBlackboardComponent();
		SetMonsterState(EMonsterState::Patrol);
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

void AMonster::M_PlayMontage_Implementation(UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	PlayAnimMontage(AnimMontage, InPlayRate, StartSectionName);
}

float AMonster::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority())
	{
		return 0.0f;
	}

	const float Damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (IsValid(StatComponent))
	{
		if (StatComponent->GetCurrentHealth() <= 0)
		{
			OnDeath();
			// Delegate Broadcasts for Achievements
			OnMonsterDead.Broadcast(DamageCauser, this);
		}
	}
	return Damage;
}

void AMonster::OnDeath()
{
	StopMovement();
	AnimInstance->StopAllMontages(1.0f);

	SetMonsterState(EMonsterState::Death);

	AIController->UnPossess();

	// Auto Destroy after 2seconds.
	SetLifeSpan(2.0f);
}

void AMonster::PlayAttackMontage()
{
	const uint8 AttackType = FMath::RandRange(0, AttackAnimations.Num() - 1);

	if (IsValid(AttackAnimations[AttackType]))
	{
		M_PlayMontage(AttackAnimations[AttackType]);
	}
}

void AMonster::StopMovement()
{
	if (AIController)
	{
		AIController->StopMovement();
	}
}

void AMonster::NotifyLightExposure(float DeltaTime, float TotalExposedTime, const FVector& PlayerLocation, AActor* PlayerActor)
{
	if (!HasAuthority()) return;

	switch (MonsterState)
	{
	case EMonsterState::Patrol:
		SetMonsterState(EMonsterState::Detected);
		break;

	case EMonsterState::Detected:
		if (TotalExposedTime < ChaseTriggerTime)
		{
			SetMonsterState(EMonsterState::Investigate);
		}
		else if (TotalExposedTime >= ChaseTriggerTime)
		{
			SetMonsterState(EMonsterState::Chase);
			AddDetection(PlayerActor);
		}
		else break;

	case EMonsterState::Investigate:
	{
		if (AIController)
		{
			if (BlackboardComponent)
			{
				BlackboardComponent->SetValueAsVector(InvestigateLocationKey, PlayerLocation);
			}
		}
		if (TotalExposedTime >= ChaseTriggerTime)
		{
			SetMonsterState(EMonsterState::Chase);
			AddDetection(PlayerActor);
		}
		break;
	}
	case EMonsterState::Chase:
		break;
	
	default:
		break;
	}
}

void AMonster::AddDetection(AActor* Actor)
{
	if (!IsValid(Actor)) return;

	int32& Count = DetectionRefCounts.FindOrAdd(Actor);
	Count++;

	// If Target is empty, set
	if (TargetActor == nullptr)
	{
		TargetActor = Actor;

		if (BlackboardComponent)
		{
			BlackboardComponent->SetValueAsObject(TargetActorKey, TargetActor);
		}
	}
}

void AMonster::RemoveDetection(AActor* Actor)
{
	if (!IsValid(Actor)) return;

	int32* CountPtr = DetectionRefCounts.Find(Actor);
	if (!CountPtr) return;

	(*CountPtr)--;

	if (*CountPtr <= 0)
	{
		DetectionRefCounts.Remove(Actor);

		if (TargetActor == Actor)
		{
			TargetActor = nullptr;

			if (BlackboardComponent)
			{
				BlackboardComponent->ClearValue(TargetActorKey);
			}
		}
	}
}


void AMonster::SetMonsterState(EMonsterState NewState)
{
	if (!HasAuthority()) return;

	if (MonsterState == NewState) return;

	MonsterState = NewState;
	UE_LOG(LogTemp, Warning, TEXT("MonsterState changed: %d �� %d"), (int32)MonsterState, (int32)NewState);

	if (UBlackboardComponent* BB = Cast<AAIController>(GetController())->GetBlackboardComponent())
	{
		BB->SetValueAsEnum(MonsterStateKey, static_cast<uint8>(NewState));
	}

	switch (NewState)
	{
	case EMonsterState::Detected:
		LOG(TEXT("AI is Detected"));
		StopMovement();
		if (IsValid(DetectedAnimations))
		{
			M_PlayMontage(DetectedAnimations);
		}
		break;

	case EMonsterState::Chase:
		LOG(TEXT("AI starts Chase"));
		SetMaxSwimSpeed(ChaseSpeed);
		bIsChasing = true;
		// @TODO : Add animations, sounds, and more
		break;

	case EMonsterState::Patrol:
		SetMaxSwimSpeed(PatrolSpeed);
		break;

	case EMonsterState::Investigate:
		SetMaxSwimSpeed(InvestigateSpeed);
		// @TODO : Add animations, sounds, and more
		break;

	default:
		break;
	}
}

void AMonster::SetMaxSwimSpeed(float Speed)
{
	GetCharacterMovement()->MaxSwimSpeed = Speed;
}



