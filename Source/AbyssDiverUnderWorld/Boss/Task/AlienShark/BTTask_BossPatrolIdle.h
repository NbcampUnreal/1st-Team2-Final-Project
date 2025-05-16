#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_BossPatrolIdle.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_BossPatrolIdle : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_BossPatrolIdle();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
};
