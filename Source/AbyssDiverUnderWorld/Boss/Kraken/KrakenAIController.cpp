#include "Boss/Kraken/KrakenAIController.h"
#include "AbyssDiverUnderWorld.h"

AKrakenAIController::AKrakenAIController()
{
}

void AKrakenAIController::BeginPlay()
{
	Super::BeginPlay();

	LOG(TEXT("KrakenAIController BeginPlay"));
}

void AKrakenAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}
