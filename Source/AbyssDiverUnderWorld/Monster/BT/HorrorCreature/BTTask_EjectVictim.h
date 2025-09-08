// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_EjectVictim.generated.h"

struct FBTEjectVictimTaskMemory
{
	TWeakObjectPtr<class AMonsterAIController> AIController;
	TWeakObjectPtr<class AMonster> Monster;
	FVector FleeLocation = FVector::ZeroVector;
	float ElapsedTime = 0.0f;
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_EjectVictim : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTTask_EjectVictim();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

#pragma region Variable
protected:
	UPROPERTY(EditAnywhere)
	float EjectTriggerDistance = 300.0f;

	// Maximum time to Eject when FleeLocation is not reached
	UPROPERTY(EditAnywhere)
	float MaxEjectDelay = 7.0f;
#pragma endregion
};
