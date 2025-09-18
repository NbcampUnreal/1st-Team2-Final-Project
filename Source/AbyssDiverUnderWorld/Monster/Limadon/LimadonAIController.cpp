#include "Monster/Limadon/LimadonAIController.h"

ALimadonAIController::ALimadonAIController()
{
	// 리마돈은 Perception 사용하지 않음.
	AIPerceptionComponent->Deactivate();
	AIPerceptionComponent->SetActive(false);
	AIPerceptionComponent->PrimaryComponentTick.bCanEverTick = false;
}

void ALimadonAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// 리마돈은 Perception 사용하지 않음.
}
