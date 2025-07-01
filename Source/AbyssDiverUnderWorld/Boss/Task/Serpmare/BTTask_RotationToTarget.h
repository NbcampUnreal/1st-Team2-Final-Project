#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_RotationToTarget.generated.h"

struct FBTRotationToTargetTask
{
	TWeakObjectPtr<class ABossAIController> AIController;
	TWeakObjectPtr<class ABoss> Boss;
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_RotationToTarget : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_RotationToTarget();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTRotationToTargetTask); }

private:
	UPROPERTY(EditAnywhere)
	float RotationInterpSpeed = 1.5f; 
	
	FName BlackboardKeyName;
	
};
