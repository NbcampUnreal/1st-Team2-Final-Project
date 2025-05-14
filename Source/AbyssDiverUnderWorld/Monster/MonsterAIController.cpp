// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/MonsterAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "Monster/TestPlayerCharacter.h"
#include "AbyssDiverUnderWorld.h"
#include "Monster/EMonsterState.h"
#include "GenericTeamAgentInterface.h"

AMonsterAIController::AMonsterAIController()
{
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SetPerceptionComponent(*AIPerceptionComponent);
	
	// Sight Filltering
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = false;
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

	if (!bIsLosingTarget) return;

	float Elapsed = GetWorld()->GetTimeSeconds() - LostTargetTime;
	if (Elapsed > SightConfig->GetMaxAge())
	{
		BlackboardComponent->ClearValue("TargetActor");
		BlackboardComponent->SetValueAsEnum("MonsterState", static_cast<uint8>(EMonsterState::Patrol));
		bIsLosingTarget = false;
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

		// Initialize BlackboardKey (CurrnetState, TargetActor)
		BlackboardComponent->SetValueAsName("CurrentState", "Patrol");
		BlackboardComponent->ClearValue("TargetActor");
	}

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
	if (Actor->IsA(ATestPlayerCharacter::StaticClass()))
	{
		BlackboardComponent = GetBlackboardComponent();

		if (Stimulus.WasSuccessfullySensed())
		{
			LOG(TEXT("[Perception] : %s"), *Actor->GetName());

			Blackboard->SetValueAsObject("TargetActor", Actor);
			Blackboard->SetValueAsEnum("MonsterState", static_cast<uint8>(EMonsterState::Chase));
		}
		else
		{
			// Lost Perception. but Target Value still remains for MaxAge
			bIsLosingTarget = true;
			LostTargetTime = GetWorld()->GetTimeSeconds(); // Timer On.
		}
	}
}


