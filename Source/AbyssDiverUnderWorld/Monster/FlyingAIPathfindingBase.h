// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Delegates/DelegateCombinations.h"
#include "FlyingAIPathfindingBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFinishedMoving);

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ABYSSDIVERUNDERWORLD_API UFlyingAIPathfindingBase : public UActorComponent
{
	GENERATED_BODY()

public:	
	UFlyingAIPathfindingBase();

protected:
	virtual void BeginPlay() override;

#pragma region Method
public:
	UFUNCTION(Server, Reliable)	
	void S_MoveTo(FVector TargetLocation);
	void S_MoveTo_Implementation(FVector TargetLocation); // RPC

	UFUNCTION(BlueprintNativeEvent)
	void MoveTo(FVector TargetLocation);
	void MoveTo_Implementation(FVector TargetLocation); // BP fallback

	UFUNCTION(BlueprintCallable)
	void TriggerMoveFinished(); // In Blueprint, Event Dispatcher Call Node

#pragma endregion
		
#pragma region Variable
public:
	UPROPERTY(BlueprintAssignable)
	FOnFinishedMoving OnFinishedMoving; 

#pragma endregion
};
