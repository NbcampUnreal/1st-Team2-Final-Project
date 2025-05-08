// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/UnitBase.h"
#include "MonsterAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Monster.generated.h"


UCLASS()
class ABYSSDIVERUNDERWORLD_API AMonster : public AUnitBase
{
	GENERATED_BODY()


public:
	AMonster();

protected:
	virtual void BeginPlay() override;

#pragma region Method
public:
	

protected:
	
private:

#pragma endregion

#pragma region Variable
public:

protected:


private:

#pragma endregion

#pragma region Getter, Setter
public:

#pragma endregion
};
