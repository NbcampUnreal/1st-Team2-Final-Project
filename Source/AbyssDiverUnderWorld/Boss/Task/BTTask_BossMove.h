#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_BossMove.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_BossMove : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_BossMove();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
};
