#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "Boss/EnhancedBossAIController.h"
#include "BTTask_MoveToLocation.generated.h"

struct FBTMoveToLocationTaskMemory
{
	/** 빙의한 AIController에 대한 참조 */
	TWeakObjectPtr<AEnhancedBossAIController> AIController;

	/** AIController의 주체에 대한 참조 */
	TWeakObjectPtr<ABoss> Boss;

	/** 현재까지 경과된 시간 */
	float AccumulatedTime;

	/** 작업이 끝나는 시간 */
	float FinishTaskInterval;

	/** 이동해야하는 위치 */
	FVector TargetLocation = FVector::ZeroVector;

	/** 이동에 방해받았을 때 차선으로 이동해야하는 위치 */
	FVector CachedLocation = FVector::ZeroVector;

	/** 이전 이동 좌표와 현재 이동 좌표의 오차를 확인하기 위한 상태 변수 */
	uint8 bHasBeenTriggeredMoveToLocation : 1;

	/** 이동에 방해받았을 때 활성화함으로써 차선위치로 이동하기 위한 상태 변수 */
	uint8 bShouldMoveToNearestPoint : 1;
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_MoveToLocation : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_MoveToLocation();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMoveToLocationTaskMemory); };
	
private:
	UPROPERTY(EditAnywhere)
	float MinFinishTaskInterval;

	UPROPERTY(EditAnywhere)
	float MaxFinishTaskInterval;
	
	UPROPERTY(EditAnywhere)
	float DecelerationTriggeredRadius;

	UPROPERTY(EditAnywhere)
	uint8 bIsInitialized : 1;

	UPROPERTY(EditAnywhere)
	float MoveSpeedMultiplier = 1.0f;
	
};
