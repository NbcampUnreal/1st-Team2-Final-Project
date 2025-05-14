// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GenericTeamAgentInterface.h"
#include "TestPlayerCharacter.generated.h"

class UAIPerceptionStimuliSourceComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API ATestPlayerCharacter : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ATestPlayerCharacter();

protected:
	virtual void BeginPlay() override;
	virtual FGenericTeamId GetGenericTeamId() const override; // Player = Team 2
#pragma region Variable
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sight")
	TObjectPtr<UAIPerceptionStimuliSourceComponent> StimuliSource;

#pragma endregion
};
