#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_PerformAttack.generated.h"

struct FBTPerformAttackMemory
{
	TWeakObjectPtr<class ABoss> Boss;
	TWeakObjectPtr<class AEnhancedBossAIController> AIController;
	float AccumulatedTime = 0.0f;
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_PerformAttack : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_PerformAttack();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTPerformAttackMemory); }

private:
	static const FName bCanAttackKey;
	
};
