// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/MonsterAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "Monster/TestPlayerCharacter.h"
#include "AbyssDiverUnderWorld.h"
#include "Monster/EMonsterState.h"
#include "Monster/Monster.h"
#include "GenericTeamAgentInterface.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "Monster/HorrorCreature/HorrorCreature.h"
#include "AbyssDiverUnderWorld.h"

AMonsterAIController::AMonsterAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SetPerceptionComponent(*AIPerceptionComponent);
	
	// Sight Filltering
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SetGenericTeamId(FGenericTeamId(1)); // Team 1 = Monster, Team 2 = Player

	AIPerceptionComponent->ConfigureSense(*SightConfig);
	AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
	bIsLosingTarget = false;
	FallbackMoveDistance = 300.0f;
}

void AMonsterAIController::BeginPlay()
{
	Super::BeginPlay();
	LoadSightDataFromTable();

	UAIPerceptionComponent* Perception = FindComponentByClass<UAIPerceptionComponent>();
	if (Perception)
	{
		Perception->OnTargetPerceptionUpdated.AddDynamic(this, &AMonsterAIController::OnTargetPerceptionUpdated);
	}
}

void AMonsterAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!IsValid(Monster) || !SightConfig) return;

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	const float MaxAge = SightConfig->GetMaxAge();

	TArray<TWeakObjectPtr<AActor>> ToRemove;

	for (const auto& Elem : LostActorsMap)
	{
		TWeakObjectPtr<AActor> Target = Elem.Key;
		float LostTime = Elem.Value;

		if (!Target.IsValid())
		{
			ToRemove.Add(Target);
			continue;
		}

		float Elapsed = CurrentTime - LostTime;
		if (Elapsed > MaxAge)
		{
			UE_LOG(LogTemp, Log, TEXT("[Elapsed] : %.2f"), Elapsed);
			Monster->ForceRemoveDetection(Target.Get());
			ToRemove.Add(Target);
		}
	}

	for (TWeakObjectPtr<AActor> Actor : ToRemove)
	{
		LostActorsMap.Remove(Actor);
	}

	// Return to state when all detectors are clear
	if (Monster->GetDetectionCount() == 0 &&
		Monster->GetMonsterState() == EMonsterState::Chase)
	{
		Monster->SetMonsterState(EMonsterState::Patrol);
		Monster->bIsChasing = false;
		LOG(TEXT("Monster State -> Patrol"));
	}
}

void AMonsterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (IsValid(BehaviorTree))
	{
		UseBlackboard(BehaviorTree->BlackboardAsset, (UBlackboardComponent*&)BlackboardComponent);
		RunBehaviorTree(BehaviorTree);
		LOG(TEXT("AIController Possess"));

		// Initialize BlackboardKey (TargetActor)
		BlackboardComponent->ClearValue("TargetActor");
	}

	Monster = Cast<AMonster>(GetPawn());

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &AMonsterAIController::InitializePatrolPoint, 0.5f, false);
}

void AMonsterAIController::LoadSightDataFromTable()
{
	if (SightDataTable == nullptr)
	{
		LOG(TEXT("SightDataTable is not assigned"));
		return;
	}

	LOG(TEXT("Loading sight data for MonsterID = %s"), *MonsterID.ToString());
	// Setting up visual information data import from a data table
	const FMonsterSightData* SightRow = SightDataTable->FindRow<FMonsterSightData>(MonsterID, TEXT("MonsterSight"));
	if (SightRow && SightConfig)
	{
		SightConfig->SightRadius = SightRow->SightRadius;
		SightConfig->LoseSightRadius = SightRow->LoseSightRadius;
		SightConfig->PeripheralVisionAngleDegrees = SightRow->PeripheralVisionAngleDegrees;
		SightConfig->SetMaxAge(SightRow->SenseInterval);

		AIPerceptionComponent->ConfigureSense(*SightConfig);
	}
	else
	{
		LOG(TEXT("No matching row found for MonsterID"));
	}
}

void AMonsterAIController::InitializePatrolPoint()
{
	if (BlackboardComponent)
	{
		BlackboardComponent->SetValueAsInt("PatrolIndex", 0);
	}
}


void AMonsterAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!IsValid(Monster)) return;

	if (Actor->IsA(AUnderwaterCharacter::StaticClass()))
	{
		if (AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(Actor))
		{
			if (Player->GetCharacterState() == ECharacterState::Death)
			{
				return;
			}
		}

 		if (Stimulus.WasSuccessfullySensed() && Monster->GetMonsterState() != EMonsterState::Flee)
		{
			LostActorsMap.Remove(Actor); // Remove when detected again
			Monster->AddDetection(Actor);

			if (Monster->GetMonsterState() != EMonsterState::Chase)
			{
				Monster->SetMonsterState(EMonsterState::Chase);
				Monster->bIsChasing = true;
			}
		}
		else
		{
			// Lost Perception. but Target Value still remains for MaxAge
			if (!LostActorsMap.Contains(Actor))
			{
				LostActorsMap.Add(Actor, GetWorld()->GetTimeSeconds());
			}
		}
	}
}

void AMonsterAIController::SetbIsLosingTarget(bool IsLosingTargetValue)
{
	if (bIsLosingTarget != IsLosingTargetValue)
	{
		bIsLosingTarget = IsLosingTargetValue;
	}
}

void AMonsterAIController::HandleMoveFailure()
{
	FVector AvoidDirection = ComputeAvoidanceDirection();
	if (!AvoidDirection.IsNearlyZero())
	{
		FVector FallbackLocation = Monster->GetActorLocation() + AvoidDirection * 300.f;
		if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
		{
			FNavLocation ProjectedLocation;
			if (NavSys->ProjectPointToNavigation(FallbackLocation, ProjectedLocation))
			{
				MoveToLocation(ProjectedLocation.Location, 100.f);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Fallback location not on NavMesh."));
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No valid avoidance direction found!"));
	}
}

FVector AMonsterAIController::ComputeAvoidanceDirection()
{
	const FVector Start = Monster->GetActorLocation();
	const FVector Forward = Monster->GetActorForwardVector();
	FVector AccumulatedDir = FVector::ZeroVector;

	for (int32 Angle = -60; Angle <= 60; Angle += 30)
	{
		FVector Dir = Forward.RotateAngleAxis(Angle, FVector::UpVector);
		FVector End = Start + Dir * 300.f;
		FHitResult Hit;
		if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
		{
			AccumulatedDir += Dir;
		}
	}

	if (AccumulatedDir.IsNearlyZero())
	{
		return -Forward; // Fallback: go backward
	}

	AccumulatedDir.Normalize();
	return AccumulatedDir;
}

void AMonsterAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	if (Result.Code != EPathFollowingResult::Success)
	{
		MoveFailCount++;
		if (MoveFailCount <= MaxMoveFailRetries)
		{
			UE_LOG(LogTemp, Warning, TEXT("Move failed. Retry %d/%d"), MoveFailCount, MaxMoveFailRetries);
			HandleMoveFailure();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Move failed too many times. Aborting fallback."));
			MoveFailCount = 0;
		}
	}
	else
	{
		MoveFailCount = 0;
	}
}