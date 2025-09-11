#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"

#include "BTTask_SetMonsterState.generated.h"

enum class EMonsterState : uint8;

/**
 * 
 */
UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_SetMonsterState : public UBTTaskNode
{
	GENERATED_BODY()

protected:

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	UPROPERTY(EditAnywhere, Category = "AI")
	EMonsterState NewState;
};
