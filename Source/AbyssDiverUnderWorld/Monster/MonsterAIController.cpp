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

const FName AMonsterAIController::TargetPlayerKey = "TargetPlayer";
const FName AMonsterAIController::bIsChasingKey = "bIsChasing";
const FName AMonsterAIController::PerceptionTypeKey = "EPerceptionType";

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

	if (AIPerceptionComponent)
	{
		AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AMonsterAIController::OnTargetPerceptionUpdated);
	}
}

void AMonsterAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
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

	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(Actor);
	if (!IsValid(Player) || Player->IsDeath() || Player->IsGroggy()) return;

	if (Actor->IsA(AUnderwaterCharacter::StaticClass()))
	{
		if (Stimulus.WasSuccessfullySensed() && Monster->GetMonsterState() != EMonsterState::Flee)
		{
			Monster->AddDetection(Actor);
		}
		else
		{
			Monster->RemoveDetection(Actor);
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

void AMonsterAIController::SetBlackboardPerceptionType(EPerceptionType InPerceptionType)
{
	BlackboardComponent->SetValueAsEnum(PerceptionTypeKey, static_cast<uint8>(InPerceptionType));
}
