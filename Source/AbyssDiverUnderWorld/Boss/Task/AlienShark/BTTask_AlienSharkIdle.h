#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_AlienSharkIdle.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_AlienSharkIdle : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_AlienSharkIdle();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
};
