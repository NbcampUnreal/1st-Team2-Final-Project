#pragma once

#include "CoreMinimal.h"
#include "AbyssDiverUnderWorld.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_SerpmareDetected.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_SerpmareDetected : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_SerpmareDetected();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Attack")
	float AttackInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Attack")
	float DetectInterval;

	UPROPERTY(EditAnywhere)
	uint8 bIsBigSerpmare : 1;
	
private:
	float AccumulatedAttackTime;
	float AccumulatedDetectTime;
};