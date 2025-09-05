#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"

#include "BTTask_Damaged.generated.h"

struct FBTDamagedTaskMemory
{
	/** 빙의한 AIController에 대한 참조 */
	TWeakObjectPtr<class AMonsterAIController> AIController;

	/** AIController의 주체에 대한 참조 */
	TWeakObjectPtr<class ABoss> Boss;

	/** 현재까지 경과된 시간 */
	float AccumulatedTime;

	/** Damaged 작업이 끝나는 시간 */
	float FinishTaskInterval;
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_Damaged : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_Damaged();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTDamagedTaskMemory); }

private:
	UPROPERTY(EditAnywhere)
	float MaxRotationTime;

	UPROPERTY(EditAnywhere)
	float MinRotationTime;

	UPROPERTY(EditAnywhere)
	float RotationStartTime;
	
};
