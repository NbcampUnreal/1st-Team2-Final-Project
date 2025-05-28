#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "Boss/EnhancedBossAIController.h"
#include "BTTask_SetMovePointAroundMonster.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_SetMovePointAroundMonster : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_SetMovePointAroundMonster();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory) override;

};
