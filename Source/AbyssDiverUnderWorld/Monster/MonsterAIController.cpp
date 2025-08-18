// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/MonsterAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/UnderwaterCharacter.h"
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

