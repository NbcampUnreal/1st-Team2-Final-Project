#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_SerpmareAttack.generated.h"

class ABoss;
class ABossAIController;

struct FBTSerpmareAttackTaskMemory
{
	TWeakObjectPtr<ABoss> Serpmare;
	TWeakObjectPtr<ABossAIController> AIController;
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_SerpmareAttack : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTTask_SerpmareAttack();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTSerpmareAttackTaskMemory); }
	
};
