#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_SetBlackboardKey.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_SetBlackboardKey : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_SetBlackboardKey();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory) override;

private:
	UPROPERTY(EditAnywhere)
	uint8 BlackBoardKeyValue : 1 = false;
	
};
