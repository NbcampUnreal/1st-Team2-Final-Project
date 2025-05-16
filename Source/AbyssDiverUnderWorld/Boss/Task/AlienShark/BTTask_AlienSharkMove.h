#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_AlienSharkMove.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_AlienSharkMove : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_AlienSharkMove();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere)
	float MoveSpeedMultiplier = 1.0f;
	
};
