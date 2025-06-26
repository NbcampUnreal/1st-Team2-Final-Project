// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_CheckTargetPath.generated.h"


UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTService_CheckTargetPath : public UBTService_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTService_CheckTargetPath();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

#pragma region Varible
public:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector MonsterStateKey;
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector TargetActorKey;
#pragma endregion
};
