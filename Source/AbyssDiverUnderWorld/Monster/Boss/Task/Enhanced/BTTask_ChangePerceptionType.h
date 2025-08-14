#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "Monster/Boss/EnhancedBossAIController.h"
#include "BTTask_ChangePerceptionType.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_ChangePerceptionType : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_ChangePerceptionType();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory) override;

private:
	UPROPERTY(EditAnywhere)
	EPerceptionType PerceptionType;
	
};
