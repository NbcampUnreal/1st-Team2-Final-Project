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

void ABoss::Move()
{
	PlayAnimMontage(MoveAnimation);
}

void ABoss::MoveStop()
{
	PlayAnimMontage(IdleAnimation);
}

void ABoss::SetMoveTimer()
{
	GetWorldTimerManager().SetTimer(MovementTimer, this, &ABoss::Move, 5.0f, false);
}

void ABoss::SetMoveStopTimer()
{
	GetWorldTimerManager().SetTimer(MovementTimer, this, &ABoss::MoveStop, 5.0f, false);
}

void ABoss::M_PlayAnimation_Implementation(class UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	PlayAnimMontage(AnimMontage, InPlayRate, StartSectionName);
}
