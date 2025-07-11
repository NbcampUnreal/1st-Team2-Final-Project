// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_EjectVictim.generated.h"


UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_EjectVictim : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTTask_EjectVictim();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

#pragma region Method
private:
	void EjectPlayerIfPossible(APawn* AIPawn);
#pragma endregion

#pragma region Variable
protected:
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector FleeLocationKey;

	UPROPERTY(EditAnywhere)
	float EjectTriggerDistance = 200.0f;

	// accumulate time when swallowed
	UPROPERTY(EditAnywhere)
	float ElapsedTime;

	// Maximum time to Eject when FleeLocation is not reached
	UPROPERTY(EditAnywhere)
	float MaxEjectDelay;
#pragma endregion
};
