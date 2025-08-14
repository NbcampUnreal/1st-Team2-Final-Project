#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_EyeStalkerDetected.generated.h"

struct FEyeStalkerDetectedMemory
{
	TWeakObjectPtr<class AEyeStalker> EyeStalker;
	TWeakObjectPtr<class AEyeStalkerAIController> AIController;
	float AccumulatedTime = 0.0f;
	uint8 bWasPlayerMoving : 1 = false;
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_EyeStalkerDetected : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTTask_EyeStalkerDetected();
	
private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FEyeStalkerDetectedMemory); }

private:
	UPROPERTY(EditAnywhere)
	float MoveDetectedTime = 5.0f;

	UPROPERTY(EditAnywhere)
	float PauseDetectedTime = 3.0f;

};
