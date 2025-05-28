#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "Boss/EnhancedBossAIController.h"
#include "BTTask_EnhancedAttack.generated.h"

struct FBTEnhancedAttackTaskMemory
{
	/** 빙의한 AIController에 대한 참조 */
	TWeakObjectPtr<AEnhancedBossAIController> AIController;

	/** AIController의 주체에 대한 참조 */
	TWeakObjectPtr<ABoss> Boss;

	/** 노드에 할당된 블랙보드 키 이름 */
	FName BlackboardKeyName;
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_EnhancedAttack : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTTask_EnhancedAttack();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTEnhancedAttackTaskMemory); }
	
};
