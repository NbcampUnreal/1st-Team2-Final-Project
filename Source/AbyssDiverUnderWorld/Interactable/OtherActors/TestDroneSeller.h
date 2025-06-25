// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ADDroneSeller.h"
#include "TestDroneSeller.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API ATestDroneSeller : public AADDroneSeller
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATestDroneSeller();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

#pragma region Method

public:

	/** 항상 드론을 활성화시킨다. */
	virtual void Interact_Implementation(AActor* InstigatorActor) override;
	
#pragma endregion
	
};
