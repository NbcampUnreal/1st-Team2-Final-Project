#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_SetNavPointAroundTarget.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_SetNavPointAroundTarget : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_SetNavPointAroundTarget();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector TargetLocationkey;

	UPROPERTY(EditAnywhere)
	float SearchRadius = 100.f;
	
};
