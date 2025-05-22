#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "Boss/EnhancedBossAIController.h"
#include "BTTask_PlayerChase.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_PlayerChase : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTTask_PlayerChase();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UPROPERTY()
	ABoss* Boss;

	UPROPERTY()
	AEnhancedBossAIController* AIController;

	UPROPERTY(EditAnywhere)
	float MoveSpeedMultiplier = 1.3f;

	UPROPERTY(EditAnywhere)
	float MaxChaseTime;

	UPROPERTY(EditAnywhere)
	float MinChaseTime;

	UPROPERTY()
	float AccumulatedTime;

	UPROPERTY()
	float TimeCriteria;
};
