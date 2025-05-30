#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_LimadonIdle.generated.h"

class ABossAIController;
class ALimadon;

struct FBTLimadonIdleTaskMemory
{
	TWeakObjectPtr<ALimadon> Limadon;
	TWeakObjectPtr<ABossAIController> AIController;
	FTimerHandle DamageHandle;
	float AccumulatedTime;
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_LimadonIdle : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_LimadonIdle();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTLimadonIdleTaskMemory); }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|AttackInterval")
	float AttackInterval;
	
};
