#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_LimadonSpit.generated.h"

class ABossAIController;
class ALimadon;

struct FBTLimadonSpitTaskMemory
{
	TWeakObjectPtr<ALimadon> Limadon;
	TWeakObjectPtr<ABossAIController> AIController;
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_LimadonSpit : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTTask_LimadonSpit();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTLimadonSpitTaskMemory); }

};
