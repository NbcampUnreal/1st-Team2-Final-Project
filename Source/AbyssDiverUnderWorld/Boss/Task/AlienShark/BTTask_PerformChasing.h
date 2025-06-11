#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_PerformChasing.generated.h"

struct FBTPerformChasingTaskMemory
{
	TWeakObjectPtr<class AEnhancedBossAIController> AIController;
	TWeakObjectPtr<class ABoss> Boss;
	float AccumulatedTime = 0.0f;
	float ChasingTime = 0.0f;
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_PerformChasing : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_PerformChasing();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTPerformChasingTaskMemory); }

private:
	UPROPERTY(EditAnywhere)
	uint8 MaxChasingTime = 20;

	UPROPERTY(EditAnywhere)
	uint8 MinChasingTime = 10;

	static const FName bCanAttackKey;
	static const FName bIsHidingKey;
	static const FName bIsPlayerHiddenKey;
	
	uint8 bHasAttacked : 1 = false;

};
