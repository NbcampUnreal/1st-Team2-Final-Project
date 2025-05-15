#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_BossMove.generated.h"

namespace EPathFollowingRequestResult
{
	enum Type : int;
}

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_BossMove : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_BossMove();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere)
	float MoveSpeedMultiplier = 1.0f;

private:
	static const FName BossStateKey;
};
