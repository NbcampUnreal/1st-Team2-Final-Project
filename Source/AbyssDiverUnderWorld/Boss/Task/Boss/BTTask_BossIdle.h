#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_BossIdle.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_BossIdle : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_BossIdle();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
};
