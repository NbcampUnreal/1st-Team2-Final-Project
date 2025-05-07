#include "Boss/Boss.h"
#include "AbyssDiverUnderWorld.h"

ABoss::ABoss()
{
	PrimaryActorTick.bCanEverTick = false;

	BlackboardComponent = nullptr;
	AIController = nullptr;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ABoss::BeginPlay()
{
	Super::BeginPlay();

	AIController = Cast<ABossAIController>(GetController());

	if (IsValid(AIController))
	{
		BlackboardComponent = AIController->GetBlackboardComponent();
		LOG(TEXT("BlackboardComponent Initialized"));
	}
}




