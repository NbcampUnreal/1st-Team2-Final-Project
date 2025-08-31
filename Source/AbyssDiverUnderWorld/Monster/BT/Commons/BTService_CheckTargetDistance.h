// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_CheckTargetDistance.generated.h"


UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTService_CheckTargetDistance : public UBTService_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTService_CheckTargetDistance();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

#pragma region Varible
public:
	UPROPERTY(EditAnywhere, Category = "AI")
	float MeleeRange = 200.0f;
	UPROPERTY(EditAnywhere, Category = "AI")
	float RangedRange = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector InMeleeRangeKey;
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector InRangedRangeKey;
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector TargetActorKey;
#pragma endregion
};
