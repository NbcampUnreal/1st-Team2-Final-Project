// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FlyingAIPathfindingBase.generated.h"


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
	void Server_MoveTo(FVector TargetLocation);
	void Server_MoveTo_Implementation(FVector TargetLocation); // RPC

	UFUNCTION(BlueprintNativeEvent)
	void MoveTo(FVector TargetLocation);
	void MoveTo_Implementation(FVector TargetLocation); // BP fallback
#pragma endregion
		
};
