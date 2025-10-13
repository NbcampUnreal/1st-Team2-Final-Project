// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/ADInGameState.h"
#include "Tutorial/TutorialEnums.h"
#include "Delegates/Delegate.h"
#include "ADTutorialGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhaseChangedDelegate, ETutorialPhase, NewPhase);

UCLASS()
class ABYSSDIVERUNDERWORLD_API AADTutorialGameState : public AADInGameState
{
	GENERATED_BODY()
	
public:
	AADTutorialGameState();

	UPROPERTY(BlueprintAssignable, Category = "Tutorial")
	FOnPhaseChangedDelegate OnPhaseChanged;

	UFUNCTION(BlueprintPure, Category = "Tutorial")
	FORCEINLINE ETutorialPhase GetCurrentPhase() const { return TutorialCurrentPhase; }

	void SetCurrentPhase(ETutorialPhase NewPhase);

	UPROPERTY(ReplicatedUsing = OnRep_PhaseChanged)
	ETutorialPhase TutorialCurrentPhase;
protected:
	UFUNCTION()
	void OnRep_PhaseChanged();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
};
