#include "Monster/Boss/BossAIController.h"

#include "AbyssDiverUnderWorld.h"
#include "Character/UnderwaterCharacter.h"

#include "Boss.h"
#include "ENum/EBossState.h"
#include "Monster/Components/AquaticMovementComponent.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

const FName ABossAIController::BossStateKey = "BossState";

ABossAIController::ABossAIController()
{
	PrimaryActorTick.bCanEverTick = false;

	DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>("DamageConfig");
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>("HearingConfig");

	MoveToLocationAcceptanceRadius = 50.0f;
	MoveToActorAcceptanceRadius = 500.0f;
	DefaultVisionAngle = 60.0f;
	ChasingVisionAngle = 45.0f;

	// 시야 설정
	SightConfig->SightRadius = 3000.0f;							// 시야 범위
	SightConfig->LoseSightRadius = 4000.0f;						// 시야 상실 범위
	SightConfig->PeripheralVisionAngleDegrees = 70.0f;			// 주변 시야 각도
	SightConfig->SetMaxAge(5.0f);								// 자극 최대 기억 시간
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	// 청각 설정
	HearingConfig->HearingRange = 10000.0f;						// 청각 범위
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	
	// 시각, 촉각 등록
	AIPerceptionComponent->ConfigureSense(*SightConfig);
	AIPerceptionComponent->ConfigureSense(*DamageConfig);
	AIPerceptionComponent->ConfigureSense(*HearingConfig);

	AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

	DetectedStateInterval = 20.0f;
}

void ABossAIController::BeginPlay()
{
	Super::BeginPlay();
}


void ABossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (IsValid(BehaviorTree))
	{
		UseBlackboard(BehaviorTree->BlackboardAsset, (UBlackboardComponent*&)BlackboardComponent);
		RunBehaviorTree(BehaviorTree);
	}

	Boss = Cast<ABoss>(InPawn);
}

void ABossAIController::SetSightRadius(float InRadius)
{
	if (IsValid(this) == false || IsPendingKillPending())
	{
		return;
	}
	
	if (!IsValid(AIPerceptionComponent)) return;
    
	UAISenseConfig_Sight* SightConfigInstance = Cast<UAISenseConfig_Sight>(
		AIPerceptionComponent->GetSenseConfig(UAISense::GetSenseID(UAISense_Sight::StaticClass()))
	);
    
	if (!SightConfigInstance) return;
    
	SightConfigInstance->SightRadius = InRadius;
	AIPerceptionComponent->ConfigureSense(*SightConfigInstance);
}

void ABossAIController::SetVisionAngle(float Angle)
{
	// Angle 수치로 시야각 전환
	UAISenseConfig_Sight* SightConfigInstance = Cast<UAISenseConfig_Sight>(AIPerceptionComponent->GetSenseConfig(UAISense::GetSenseID(UAISense_Sight::StaticClass())));
	SightConfigInstance->PeripheralVisionAngleDegrees = Angle;
	AIPerceptionComponent->ConfigureSense(*SightConfigInstance);
}

bool ABossAIController::IsStateSame(EBossState State)
{
	return (GetBlackboardComponent()->GetValueAsEnum(BossStateKey) == static_cast<uint8>(State));
}

void ABossAIController::MoveToActorWithRadius(AActor* TargetActor)
{
	if (IsValid(TargetActor) == false)
	{
		LOGV(Error, TEXT("TargetActor is not valid"));
		return;
	}

	MoveToLocationWithRadius(TargetActor->GetActorLocation());
}

void ABossAIController::MoveToLocationWithRadius(const FVector& Location)
{
	ABoss* BossCharacter = Cast<ABoss>(GetPawn());
	if (IsValid(BossCharacter) == false)
	{
		LOGV(Error, TEXT("BossCharacter is not valid"));
		return;
	}

	BossCharacter->AquaticMovementComponent->SetTargetLocation(Location, MoveToActorAcceptanceRadius);
}
