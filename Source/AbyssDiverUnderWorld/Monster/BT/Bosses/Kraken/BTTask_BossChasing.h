#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"

#include "BTTask_BossChasing.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_BossChasing : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_BossChasing();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere)
	float MoveSpeedMultiplier = 1.3f;
	
	UPROPERTY(EditAnywhere)
	float TickInterval = 1.f;
	
private:
	static const FName bCanAttackKey;
	static const FName bIsChasingKey;
	
	float AccumulatedTime = 0.0f;
};
