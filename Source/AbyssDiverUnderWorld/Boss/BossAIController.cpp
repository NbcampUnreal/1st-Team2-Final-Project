#include "Boss/BossAIController.h"

#include "AbyssDiverUnderWorld.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

ABossAIController::ABossAIController()
{
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>("PerceptionComponent");
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>("SightConfig");

	// 시야 설정
	SightConfig->SightRadius = 1500.0f;					// 시야 범위
	SightConfig->LoseSightRadius = 2000.0f;				// 시야 상실 범위
	SightConfig->PeripheralVisionAngleDegrees = 70.0f;	// 주변 시야 각도
	SightConfig->SetMaxAge(5.0f);						// 자극 최대 기억 시간

	// 감지 설정
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = false;

	// 시각을 우선순위로 설정
	AIPerceptionComponent->ConfigureSense(*SightConfig);
	AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
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

		LOG(TEXT("AIController Possess"));
	}
}
