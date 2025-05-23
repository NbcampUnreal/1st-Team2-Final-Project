#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "Boss/EnhancedBossAIController.h"
#include "BTTask_EnhancedIdle.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_EnhancedIdle : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_EnhancedIdle();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory) override;

private:
	UPROPERTY()
	TObjectPtr<ABoss> Boss;

	UPROPERTY()
	TObjectPtr<AEnhancedBossAIController> AIController;
	
};
