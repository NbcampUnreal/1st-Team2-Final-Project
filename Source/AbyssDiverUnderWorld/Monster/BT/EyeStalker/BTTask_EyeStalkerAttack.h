#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"

#include "BTTask_EyeStalkerAttack.generated.h"

struct FEyeStalkerAttackMemory
{
	TWeakObjectPtr<class AEyeStalker> EyeStalker;
	TWeakObjectPtr<class AEyeStalkerAIController> AIController;
	float AccumulatedTime = 0.0f;
	uint8 bWasPlayerMoving : 1 = false;
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_EyeStalkerAttack : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_EyeStalkerAttack();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FEyeStalkerAttackMemory); }

private:
	UPROPERTY(EditAnywhere)
	float PauseDetectedTime = 3.0f;

	UPROPERTY(meta = (ClampMin = "0"))
	uint8 EyeStalkerReferenceCount = 0;
};
