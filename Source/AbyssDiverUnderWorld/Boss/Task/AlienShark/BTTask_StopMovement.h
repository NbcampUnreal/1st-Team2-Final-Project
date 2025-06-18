#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_StopMovement.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_StopMovement : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_StopMovement();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory) override;
	
};
