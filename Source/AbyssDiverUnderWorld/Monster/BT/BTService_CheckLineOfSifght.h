// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_CheckLineOfSifght.generated.h"


UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTService_CheckLineOfSifght : public UBTService_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTService_CheckLineOfSifght();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
#pragma region Variable

private:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector bInLineOfSightKey;
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector TargetActorKey;

};
