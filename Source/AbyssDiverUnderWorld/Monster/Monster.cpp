// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Monster.h"
#include "Components/SplineComponent.h"
#include "Monster/SplinePathActor.h"
#include "Character/StatComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbyssDiverUnderWorld.h"
#include "Components/CapsuleComponent.h"
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
	ChaseTriggerTime = 1.8f;
	ChaseSpeed = 400.0f;
	PatrolSpeed = 200.0f;
	InvestigateSpeed = 300.0f;
	bIsChasing = false;

	bUseControllerRotationYaw = false;
	bReplicates = true;
	SetReplicatingMovement(true);
	
	// Set Default PossessSetting
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	// Set Collision Channel == Monster
	GetCapsuleComponent()->SetCollisionObjectType(ECC_GameTraceChannel3);

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
		MovementComp->RotationRate = FRotator(0.0f, 90.0f, 0.0f);
	}
}

void AMonster::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMonster, MonsterState);
	DOREPLIFETIME(AMonster, bIsChasing);
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

	UAnimMontage* SelectedMontage = AttackAnimations[AttackType];

	if (IsValid(SelectedMontage))
	{
		UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
		if (AnimInst && !AnimInst->Montage_IsPlaying(SelectedMontage))
		{
			M_PlayMontage(SelectedMontage);
		}
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
	if (!IsValid(this)) return;
	if (AIController == nullptr) return;

	UE_LOG(LogTemp, Warning, TEXT("TotalExposedTime : %.2f"), TotalExposedTime);
	switch (MonsterState)
	{
	case EMonsterState::Patrol:
		SetMonsterState(EMonsterState::Detected);
		break;

	case EMonsterState::Detected:
		if (TotalExposedTime >= ChaseTriggerTime)
		{
			AddDetection(PlayerActor);
			SetMonsterState(EMonsterState::Chase);
		}
		else
		{
			SetMonsterState(EMonsterState::Investigate);
		}
		break;

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
		else if (TotalExposedTime < ChaseTriggerTime)
		{
			if (TargetActor == nullptr)
			{
				SetMonsterState(EMonsterState::Investigate);
			}
			else
			{
				SetMonsterState(EMonsterState::Chase);
			}
		}
		break;
	}
	case EMonsterState::Chase:
		if (TargetActor != PlayerActor)
		{
			AddDetection(PlayerActor);
		}
		break;

	case EMonsterState::Flee:
		break;
	
	default:
		break;
	}
}

void AMonster::AddDetection(AActor* Actor)
{
	if (!IsValid(Actor) || !IsValid(this)) return;

	int32& Count = DetectionRefCounts.FindOrAdd(Actor);
	Count++;

	UE_LOG(LogTemp, Log, TEXT("[%s] AddDetection : %s | Count: %d"),
		*GetName(),
		*Actor->GetName(),
		Count
	);

	// If Target is empty, set
	if (TargetActor == nullptr || !IsValid(TargetActor))
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
	if (!IsValid(this)) return;

	if (!IsValid(Actor))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] RemoveDetection : Invalid Player! Force remove."), *GetName());
		DetectionRefCounts.Remove(Actor);
		return;
	}

	int32* CountPtr = DetectionRefCounts.Find(Actor);
	if (!CountPtr) return;

	(*CountPtr)--;

	UE_LOG(LogTemp, Log, TEXT("[%s] RemoveDetection : %s | New Count: %d"),
		*GetName(),
		*Actor->GetName(),
		*CountPtr
	);

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

			// Alternate targeting
			for (const auto& Pair : DetectionRefCounts)
			{
				if (IsValid(Pair.Key))
				{
					TargetActor = Pair.Key;
					if (BlackboardComponent)
					{
						BlackboardComponent->SetValueAsObject(TargetActorKey, TargetActor);
					}
					UE_LOG(LogTemp, Log, TEXT("[%s] New TargetActor: %s"), *GetName(), *TargetActor->GetName());

					break;
				}
			}
		}
	}
}

/**
Quick fix : Because it is a 3D space, RefCount sometimes goes up abnormally when AddDetection is perceived by Perception.
Therefore, when the TargetActor is lost for more than LoseRadius && MaxAge time after Perception.
Methods to bring down abnormally high RefCount at once 
**/
void AMonster::ForceRemoveDetection(AActor* Actor)
{
	if (!IsValid(this)) return;

	if (!IsValid(Actor))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] RemoveDetection : Invalid Player! Force remove."), *GetName());
		DetectionRefCounts.Remove(Actor);
		return;
	}

	int32* CountPtr = DetectionRefCounts.Find(Actor);
	if (CountPtr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ForceRemoveDetection] Actor: %s, OldCount: %d -> 0"),
			*Actor->GetName(), *CountPtr);

		*CountPtr = 0;
		RemoveDetection(Actor);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ForceRemoveDetection] Actor: %s not found in DetectionRefCounts"),
			*Actor->GetName());
	}

	if (TargetActor == Actor)
	{
		TargetActor = nullptr;

		if (BlackboardComponent)
		{
			BlackboardComponent->ClearValue(TargetActorKey);
		}

		// Alternate targeting
		for (const auto& Pair : DetectionRefCounts)
		{
			if (IsValid(Pair.Key))
			{
				TargetActor = Pair.Key;
				if (BlackboardComponent)
				{
					BlackboardComponent->SetValueAsObject(TargetActorKey, TargetActor);
				}
				break;
			}
		}
	}
}


void AMonster::SetMonsterState(EMonsterState NewState)
{
	if (!HasAuthority()) return;

	if (MonsterState == NewState) return;

	if (GetController() == nullptr) return;

	MonsterState = NewState;
	UE_LOG(LogTemp, Warning, TEXT("MonsterState changed: %d -> %d"), (int32)MonsterState, (int32)NewState);

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
		if (UBlackboardComponent* BB = Cast<AAIController>(GetController())->GetBlackboardComponent())
		{
			BB->ClearValue(InvestigateLocationKey);
		}
		bIsChasing = false;
		break;

	case EMonsterState::Investigate:
		SetMaxSwimSpeed(InvestigateSpeed);
		bIsChasing = false;
		// @TODO : Add animations, sounds, and more
		break;

	case EMonsterState::Flee:
		SetMaxSwimSpeed(FleeSpeed);
		bIsChasing = false;

	default:
		break;
	}
}

void AMonster::SetMaxSwimSpeed(float Speed)
{
	GetCharacterMovement()->MaxSwimSpeed = Speed;
}

int32 AMonster::GetDetectionCount() const
{
	int32 ValidCount = 0;

	for (const auto& Elem : DetectionRefCounts)
	{
		if (Elem.Value > 0 && IsValid(Elem.Key))
		{
			++ValidCount;
		}
	}

	return ValidCount;
}



