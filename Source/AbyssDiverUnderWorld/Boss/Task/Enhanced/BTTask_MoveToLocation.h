#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "Boss/EnhancedBossAIController.h"
#include "BTTask_MoveToLocation.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_MoveToLocation : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_MoveToLocation();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UPROPERTY()
	TObjectPtr<ABoss> Boss;

	UPROPERTY()
	TObjectPtr<AEnhancedBossAIController> AIController;

	UPROPERTY()
	FVector TargetLocation;

	UPROPERTY(EditAnywhere)
	float MoveSpeedMultiplier = 1.0f;
	
};
