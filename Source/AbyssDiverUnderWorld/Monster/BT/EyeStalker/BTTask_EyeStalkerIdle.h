#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_EyeStalkerIdle.generated.h"

struct FEyeStalkerIdleMemory
{
	TWeakObjectPtr<class AEyeStalker> EyeStalker;
	TWeakObjectPtr<class AEyeStalkerAIController> AIController;
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_EyeStalkerIdle : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_EyeStalkerIdle();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FEyeStalkerIdleMemory); }
	
};
