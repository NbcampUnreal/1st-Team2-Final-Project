// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/MonsterAIController.h"

#include "AbyssDiverUnderWorld.h"
#include "Character/UnderwaterCharacter.h"
#include "Container/BlackboardKeys.h"

#include "Monster/EMonsterState.h"
#include "Monster/Monster.h"
#include "Monster/Components/AquaticMovementComponent.h"
#include "Monster/HorrorCreature/HorrorCreature.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "GenericTeamAgentInterface.h"
#include "Perception/AISenseConfig_Hearing.h"

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
}

void AMonsterAIController::MoveToActorWithRadius(AActor* TargetActor)
{
	if (IsValid(TargetActor) == false)
	{
		LOGV(Error, TEXT("TargetActor is not valid"));
		return;
	}

	MoveToLocationWithRadius(TargetActor->GetActorLocation());
}

void AMonsterAIController::MoveToLocationWithRadius(const FVector& Location)
{
	AMonster* MonsterCharacter = Cast<AMonster>(GetPawn());
	if (IsValid(MonsterCharacter) == false)
	{
		LOGV(Error, TEXT("MonsterCharacter is not valid"));
		return;
	}

	MonsterCharacter->AquaticMovementComponent->SetTargetLocation(Location, MoveToActorAcceptanceRadius);
}

void AMonsterAIController::BeginPlay()
{
	Super::BeginPlay();
	LoadSightDataFromTable();

	if (AIPerceptionComponent)
	{
		AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AMonsterAIController::OnTargetPerceptionUpdated);
		AIPerceptionComponent->OnTargetPerceptionForgotten.AddDynamic(this, &AMonsterAIController::OnTargetPerceptionForgotten);
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

		BlackboardComponent->ClearValue(BlackboardKeys::TargetPlayerKey);
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
		SightConfig->PointOfViewBackwardOffset = SightRow->PointOfViewBackwardOffset;

		AIPerceptionComponent->ConfigureSense(*SightConfig);
	}
	else
	{
		LOG(TEXT("No matching row found for MonsterID"));
	}
}

void AMonsterAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	AMonster* OwningMonster = GetOwningMonster();
	if (!OwningMonster) return;

	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(Actor);
	if (!IsValid(Player) || Player->IsDeath() || Player->IsGroggy()) return;

	if (Stimulus.WasSuccessfullySensed() && OwningMonster->GetMonsterState() != EMonsterState::Flee)
	{
		OwningMonster->AddDetection(Actor);
	}
	else
	{
		OwningMonster->RemoveDetection(Actor);
	}
}

void AMonsterAIController::OnTargetPerceptionForgotten(AActor* ForgottenActor)
{
	AMonster* OwningMonster = GetOwningMonster();
	if (!OwningMonster) return;

	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(ForgottenActor);
	if (!IsValid(Player)) return;

	// 가끔 Perception이 해제되지 않는 버그가 있는 것 같아서 Forget 기능을 추가
	// Forget시 해당 액터 감지 해제
	OwningMonster->RemoveDetection(ForgottenActor);
}

void AMonsterAIController::SetBlackboardPerceptionType(EPerceptionType InPerceptionType)
{
	BlackboardComponent->SetValueAsEnum(BlackboardKeys::PerceptionTypeKey, static_cast<uint8>(InPerceptionType));
}

bool AMonsterAIController::IsStateSame(EMonsterState State)
{
	return (GetBlackboardComponent()->GetValueAsEnum(BlackboardKeys::MonsterStateKey) == static_cast<uint8>(State));
}

float AMonsterAIController::GetSightRadius() const
{
	if (!IsValid(AIPerceptionComponent))
	{
		LOGV(Error, TEXT("AIPerceptionComponent is Invalid, return 0.0f"));
		return 0.0f;
	}

	const UAISenseConfig_Sight* SightConfigInstance = Cast<UAISenseConfig_Sight>(AIPerceptionComponent->GetSenseConfig(UAISense::GetSenseID(UAISense_Sight::StaticClass())));
	if (!SightConfigInstance)
	{
		LOGV(Error, TEXT("SightConfigInstance is Invalid, return 0.0f"));
		return 0.0f;
	}

	return SightConfigInstance->SightRadius;
}

float AMonsterAIController::GetLoseSightRadius() const
{
	if (!IsValid(AIPerceptionComponent))
	{
		LOGV(Error, TEXT("AIPerceptionComponent is Invalid, return 0.0f"));
		return 0.0f;
	}

	const UAISenseConfig_Sight* SightConfigInstance = Cast<UAISenseConfig_Sight>(AIPerceptionComponent->GetSenseConfig(UAISense::GetSenseID(UAISense_Sight::StaticClass())));
	if (!SightConfigInstance)
	{
		LOGV(Error, TEXT("SightConfigInstance is Invalid, return 0.0f"));
		return 0.0f;
	}

	return SightConfigInstance->LoseSightRadius;
}

float AMonsterAIController::GetHearingRadius() const
{
	if (!IsValid(AIPerceptionComponent))
	{
		LOGV(Error, TEXT("AIPerceptionComponent is Invalid, return 0.0f"));
		return 0.0f;
	}

	const UAISenseConfig_Hearing* HearingConfigInstance = Cast<UAISenseConfig_Hearing>(AIPerceptionComponent->GetSenseConfig(UAISense::GetSenseID(UAISenseConfig_Hearing::StaticClass())));
	if (!HearingConfigInstance)
	{
		LOGV(Log, TEXT("HearingConfigInstance is Invalid, return 0.0f"));
		return 0.0f;
	}

	return HearingConfigInstance->HearingRange;
}

AMonster* AMonsterAIController::GetOwningMonster()
{
	if (Monster.IsValid())
	{
		return Monster.Get();
	}

	Monster = Cast<AMonster>(GetPawn());
	return Monster.Get();
}
