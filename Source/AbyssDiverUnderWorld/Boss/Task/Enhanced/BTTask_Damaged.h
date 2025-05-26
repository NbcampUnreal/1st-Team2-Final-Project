#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "Boss/EnhancedBossAIController.h"
#include "BTTask_Damaged.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_Damaged : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_Damaged();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

private:
	UPROPERTY()
	TObjectPtr<ABoss> Boss;

	UPROPERTY()
	TObjectPtr<AEnhancedBossAIController> AIController;

	UPROPERTY(EditAnywhere)
	float MaxRotationTime;

	UPROPERTY(EditAnywhere)
	float MinRotationTime;

	UPROPERTY(EditAnywhere)
	float RotationStartTime;

	UPROPERTY()
	float AccumulatedTime;

	UPROPERTY()
	float TimeCriteria;
	
};
