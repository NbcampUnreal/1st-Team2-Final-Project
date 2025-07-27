// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Tutorial/TutorialEnums.h"
#include "ADTutorialGameState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPhaseChangedDelegate, ETutorialPhase);

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADTutorialGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	AADTutorialGameState();

	FOnPhaseChangedDelegate OnPhaseChanged;

	FORCEINLINE ETutorialPhase GetCurrentPhase() const { return CurrentPhase; }

	void SetCurrentPhase(ETutorialPhase NewPhase);

	UPROPERTY(ReplicatedUsing = OnRep_PhaseChanged)
	ETutorialPhase CurrentPhase;
protected:
	UFUNCTION()
	void OnRep_PhaseChanged();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
};
