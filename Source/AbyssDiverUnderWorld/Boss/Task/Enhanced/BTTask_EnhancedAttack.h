#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "Boss/EnhancedBossAIController.h"
#include "BTTask_EnhancedAttack.generated.h"

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_EnhancedAttack : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTTask_EnhancedAttack();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds) override;

	UFUNCTION()
	void FinishPerception(UAnimMontage* Montage, bool bInterrupted);

private:
	UPROPERTY()
	ABoss* Boss;

	UPROPERTY()
	AEnhancedBossAIController* AIController;

	UPROPERTY()
	UAnimInstance* AnimInstance;

	UPROPERTY()
	uint8 bHasAttacked : 1;
	
};
