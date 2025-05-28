#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "Boss/EnhancedBossAIController.h"
#include "BTTask_EnhancedIdle.generated.h"

struct FBTIdleTaskMemory
{
	/** 빙의한 AIController에 대한 참조 */
	TWeakObjectPtr<AEnhancedBossAIController> AIController;

	/** AIController의 주체에 대한 참조 */
	TWeakObjectPtr<ABoss> Boss;
	
	/** 현재까지 경과된 시간 */
	float AccumulatedTime;

	/** Idle 작업이 끝나는 시간 */
	float IdleFinishTime;
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_EnhancedIdle : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_EnhancedIdle();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTIdleTaskMemory); }
	
private:
	UPROPERTY(EditAnywhere)
	float IdleFinishMinInterval;

	UPROPERTY(EditAnywhere)
	float IdleFinishMaxInterval;
	
};
