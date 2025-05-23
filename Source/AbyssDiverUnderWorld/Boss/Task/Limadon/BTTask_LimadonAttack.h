#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_LimadonAttack.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_LimadonAttack : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_LimadonAttack();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	static const FName BossStateKey;
	
};
