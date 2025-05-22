#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "Boss/EnhancedBossAIController.h"
#include "BTTask_BloodDetected.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_BloodDetected : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_BloodDetected();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UPROPERTY()
	ABoss* Boss;

	UPROPERTY()
	AEnhancedBossAIController* AIController;
};
