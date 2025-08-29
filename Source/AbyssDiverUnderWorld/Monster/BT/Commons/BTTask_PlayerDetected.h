#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "Monster/Boss/EnhancedBossAIController.h"
#include "BTTask_PlayerDetected.generated.h"

struct FBTPlayerDetectedTaskMemory
{
	/** 빙의한 AIController에 대한 참조 */
	TWeakObjectPtr<AEnhancedBossAIController> AIController;

	/** AIController의 주체에 대한 참조 */
	TWeakObjectPtr<ABoss> Boss;

	/** 현재까지 경과된 시간 */
	float AccumulatedTime;
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_PlayerDetected : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_PlayerDetected();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTPlayerDetectedTaskMemory); }
	
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	float DetectedStateInterval;
	
};
