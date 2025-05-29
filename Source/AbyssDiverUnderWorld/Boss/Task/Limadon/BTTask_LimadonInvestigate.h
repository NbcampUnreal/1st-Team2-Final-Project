#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_LimadonInvestigate.generated.h"

class ABossAIController;
class ALimadon;

struct FBTLimadonInvestigateTaskMemory
{
	TWeakObjectPtr<ALimadon> Limadon;
	TWeakObjectPtr<ABossAIController> AIController;
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_LimadonInvestigate : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_LimadonInvestigate();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
