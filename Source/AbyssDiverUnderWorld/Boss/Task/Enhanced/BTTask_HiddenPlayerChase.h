#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "Boss/EnhancedBossAIController.h"
#include "BTTask_HiddenPlayerChase.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_HiddenPlayerChase : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_HiddenPlayerChase();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UPROPERTY()
	ABoss* Boss;

	UPROPERTY()
	AEnhancedBossAIController* AIController;
	
};
