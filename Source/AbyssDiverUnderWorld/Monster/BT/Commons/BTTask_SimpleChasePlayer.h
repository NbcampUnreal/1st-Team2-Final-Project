#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"

#include "BTTask_SimpleChasePlayer.generated.h"

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_SimpleChasePlayer : public UBTTaskNode
{
	GENERATED_BODY()
	
public:

	UBTTask_SimpleChasePlayer();

protected:

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
