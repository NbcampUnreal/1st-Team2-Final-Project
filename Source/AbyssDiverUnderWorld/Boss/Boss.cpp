#include "Boss/Boss.h"
#include "AbyssDiverUnderWorld.h"

ABoss::ABoss()
{
	PrimaryActorTick.bCanEverTick = false;
	
	BlackboardComponent = nullptr;
	AIController = nullptr;
	TargetPlayer = nullptr;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ABoss::BeginPlay()
{
	Super::BeginPlay();

	AIController = Cast<ABossAIController>(GetController());

	if (IsValid(AIController))
	{
		BlackboardComponent = AIController->GetBlackboardComponent();
	}
}

void ABoss::Move()
{
	M_PlayAnimation(MoveAnimation);
}

void ABoss::MoveStop()
{
	M_PlayAnimation(IdleAnimation);
}

void ABoss::MoveToTarget()
{
	
}

void ABoss::MoveToLastDetectedLocation()
{

}

void ABoss::M_PlayAnimation_Implementation(class UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	PlayAnimMontage(AnimMontage, InPlayRate, StartSectionName);
}

APawn* ABoss::GetTarget()
{
	if (IsValid(TargetPlayer)) 
	{
		return TargetPlayer;
	}
	
	return nullptr;
}

void ABoss::SetTarget(APawn* Target)
{
	if (IsValid(Target))
	{
		LOG(TEXT(" [Boss] Target Player is already set."));
		TargetPlayer = Target;
	}
}

void ABoss::SetLastDetectedLocation(const FVector& InLastDetectedLocation)
{
	LastDetectedLocation = InLastDetectedLocation;
}
