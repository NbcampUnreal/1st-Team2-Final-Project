#include "Monster/Serpmare/SerpmareAIController.h"

#include "AbyssDiverUnderWorld.h"

#include "Monster/Serpmare/Serpmare.h"

#include "Components/CapsuleComponent.h"

ASerpmareAIController::ASerpmareAIController()
{
	// Serpmare는 Perception 사용하지 않음.
	//AIPerceptionComponent->Deactivate();
	//AIPerceptionComponent->SetActive(false);
	//AIPerceptionComponent->PrimaryComponentTick.bCanEverTick = false;
}

void ASerpmareAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (SightConfig == nullptr)
	{
		LOGV(Error, TEXT("SightConfig == nullptr"));
		return;
	}

	ASerpmare* Serpmare = Cast<ASerpmare>(InPawn);
	if (Serpmare == nullptr)
	{
		LOGV(Error, TEXT("Serpmare == nullptr"));
		return;
	}

	UCapsuleComponent* AttackCollision = Serpmare->AttackCollision;
	if (AttackCollision == nullptr)
	{
		LOGV(Error, TEXT("AttackCollision == nullptr"));
		return;
	}

	SightConfig->SightRadius = FMath::Max(AttackCollision->GetScaledCapsuleHalfHeight(), AttackCollision->GetScaledCapsuleRadius());
	SightConfig->LoseSightRadius = SightConfig->SightRadius;
	SightConfig->PeripheralVisionAngleDegrees = 180.0f;
	SightConfig->PointOfViewBackwardOffset = 0;
	SightConfig->SetMaxAge(1.0f);
	AIPerceptionComponent->ConfigureSense(*SightConfig);
}

void ASerpmareAIController::BeginPlay()
{
	Super::BeginPlay();

	
}

void ASerpmareAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	Super::OnTargetPerceptionUpdated(Actor, Stimulus);
}
