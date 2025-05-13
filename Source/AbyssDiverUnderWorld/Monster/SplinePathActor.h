// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SplinePathActor.generated.h"

class USplineComponent;

UCLASS()
class ABYSSDIVERUNDERWORLD_API ASplinePathActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ASplinePathActor();

#pragma region Variable
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USplineComponent> SplineComponent;

#pragma endregion

#pragma region Getter, Setter
public:
	UFUNCTION(BlueprintCallable)
	USplineComponent* GetSplineComponent() const { return SplineComponent; }

#pragma endregion
};
