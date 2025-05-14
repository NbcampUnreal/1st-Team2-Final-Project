#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_SerpmareIdle.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_SerpmareIdle : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_SerpmareIdle();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
};
