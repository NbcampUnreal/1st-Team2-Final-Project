#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"

#include "BTTask_PlayerChase.generated.h"

struct FBTPlayerChaseTaskMemory
{
	/** 빙의한 AIController에 대한 참조 */
	TWeakObjectPtr<class AMonsterAIController> AIController;

	/** AIController의 주체에 대한 참조 */
	TWeakObjectPtr<class AMonster> Monster;

	/** Chase 작업이 끝나는 시간 */
	float FinishTaskInterval;
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_PlayerChase : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTTask_PlayerChase();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTPlayerChaseTaskMemory); }

private:
	UPROPERTY(EditAnywhere)
	float ChaseMoveSpeed;

	UPROPERTY(EditAnywhere)
	float ChaseDeceleration;

	UPROPERTY(EditAnywhere)
	float MaxChaseTime;

	UPROPERTY(EditAnywhere)
	float MinChaseTime;

	//EPathFollowingRequestResult::Type Result;
};
