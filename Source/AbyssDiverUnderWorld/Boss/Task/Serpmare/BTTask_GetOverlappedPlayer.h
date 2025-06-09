#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_GetOverlappedPlayer.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_GetOverlappedPlayer : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_GetOverlappedPlayer();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory) override;
	
};
