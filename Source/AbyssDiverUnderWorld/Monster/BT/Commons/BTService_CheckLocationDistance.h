// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_CheckLocationDistance.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTService_CheckLocationDistance : public UBTService_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTService_CheckLocationDistance();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

#pragma region Varible
public:
	UPROPERTY(EditAnywhere, Category = "AI")
	float AcceptRadius = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector TargetLocationKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector ReturnBoolKey;
};
