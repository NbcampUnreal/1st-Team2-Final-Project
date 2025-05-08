#include "Boss/Boss.h"
#include "AbyssDiverUnderWorld.h"

ABoss::ABoss()
{
	PrimaryActorTick.bCanEverTick = false;
	
	BlackboardComponent = nullptr;
	AIController = nullptr;
	TargetPlayer = nullptr;
	LastDetectedLocation = FVector::ZeroVector;

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

void ABoss::RotationToTarget()
{
	if (!IsValid(TargetPlayer)) return;

	const FRotator CurrentRotation = GetActorRotation();
	const FRotator TargetRotation = (TargetPlayer->GetActorLocation() - GetActorLocation()).Rotation();
	const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, GetWorld()->GetDeltaSeconds(), 2.0f);
	
	SetActorRotation(NewRotation);
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