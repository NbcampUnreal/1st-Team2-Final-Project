#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "Boss/EnhancedBossAIController.h"
#include "BTTask_EnhancedMove.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_EnhancedMove : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_EnhancedMove();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UPROPERTY()
	ABoss* Boss;

	UPROPERTY()
	AEnhancedBossAIController* AIController;

	UPROPERTY(EditAnywhere)
	float MoveSpeedMultiplier = 1.0f;

	UPROPERTY()
	FVector PatrolPoint;
	
};
