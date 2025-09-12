#include "Monster/Serpmare/SerpmareAIController.h"

ASerpmareAIController::ASerpmareAIController()
{
	// Serpmare는 Perception 사용하지 않음.
	AIPerceptionComponent->Deactivate();
	AIPerceptionComponent->SetActive(false);
	AIPerceptionComponent->PrimaryComponentTick.bCanEverTick = false;
}

void ASerpmareAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// Serpmare는 Perception 사용하지 않음.
}
