// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_MoveToNavmeshZone.generated.h"


UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_MoveToNavmeshZone : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
    UBTTask_MoveToNavmeshZone();

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

#pragma region Variable
public:
    UPROPERTY(EditAnywhere)
    FBlackboardKeySelector ReturnToNavMeshLocationKey;

    UPROPERTY(EditAnywhere)
    float AcceptanceRadius = 50.f;

#pragma endregion
};
