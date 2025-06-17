// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/MonsterAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "Monster/TestPlayerCharacter.h"
#include "AbyssDiverUnderWorld.h"
#include "Monster/EMonsterState.h"
#include "Monster/Monster.h"
#include "GenericTeamAgentInterface.h"
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

	if (!bIsLosingTarget || !IsValid(Monster)) return;

	float Elapsed = GetWorld()->GetTimeSeconds() - LostTargetTime;
	if (Elapsed > SightConfig->GetMaxAge())
	{
		bIsLosingTarget = false;
		Monster->SetMonsterState(EMonsterState::Patrol);
		Monster->bIsChasing = false;
		LOG(TEXT("Monster State : Patrol"));
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
		BlackboardComponent = GetBlackboardComponent();

 		if (Stimulus.WasSuccessfullySensed() && Monster->GetMonsterState() != EMonsterState::Flee)
		{
			Monster->AddDetection(Actor);
			bIsLosingTarget = false;

			if (Monster->GetMonsterState() != EMonsterState::Chase)
			{
				Monster->SetMonsterState(EMonsterState::Chase);
				Monster->bIsChasing = true;
			}
		}
		else
		{
			// Lost Perception. but Target Value still remains for MaxAge
			bIsLosingTarget = true;
			Monster->RemoveDetection(Actor);
			LostTargetTime = GetWorld()->GetTimeSeconds(); // Timer On.
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


