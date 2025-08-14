#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_SerpmareIdle.generated.h"

class ASerpmare;
class ABossAIController;

struct FBTSerpmareIdleTaskMemory
{
	TWeakObjectPtr<ASerpmare> Serpmare;
	TWeakObjectPtr<ABossAIController> AIController;
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_SerpmareIdle : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_SerpmareIdle();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTSerpmareIdleTaskMemory); }

protected:
	UPROPERTY(EditAnywhere)
	uint8 bIsBigSerpmare : 1;
	
};
