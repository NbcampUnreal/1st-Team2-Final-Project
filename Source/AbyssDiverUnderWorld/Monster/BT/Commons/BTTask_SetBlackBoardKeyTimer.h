#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_SetBlackBoardKeyTimer.generated.h"

struct FBTSetBlackBoardKeyTimerTask
{
	TWeakObjectPtr<class ABoss> Boss;
	TWeakObjectPtr<class AMonsterAIController> AIController;
	float AccumulatedTime = 0.0f;
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_SetBlackBoardKeyTimer : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_SetBlackBoardKeyTimer();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTSetBlackBoardKeyTimerTask); }

private:
	UPROPERTY(EditAnywhere)
	float ChangeBlackboardKeyTime = 2.0f;

	UPROPERTY(EditAnywhere)
	uint8 bBlackboardKeyValue : 1 = false;

	FName BlackboardKeyName;
	
};
