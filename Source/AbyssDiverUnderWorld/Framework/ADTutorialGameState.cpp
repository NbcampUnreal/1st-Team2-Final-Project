// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/ADTutorialGameState.h"
#include "Net/UnrealNetwork.h"

AADTutorialGameState::AADTutorialGameState()
{
	CurrentPhase = ETutorialPhase::None;
}

void AADTutorialGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AADTutorialGameState, CurrentPhase);
}

void AADTutorialGameState::SetCurrentPhase(ETutorialPhase NewPhase)
{
	if (HasAuthority())
	{
		if (CurrentPhase != NewPhase)
		{
			CurrentPhase = NewPhase;
			OnRep_PhaseChanged();
		}
	}
}

void AADTutorialGameState::OnRep_PhaseChanged()
{
	OnPhaseChanged.Broadcast(CurrentPhase);
}
