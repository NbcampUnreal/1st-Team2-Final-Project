#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_HealthRecovery.generated.h"

struct FBTHealthRecoveryMemory
{
	/** AIController에 대한 참조 */
	TWeakObjectPtr<class AMonsterAIController> AIController;

	/** AIController의 주체에 대한 참조 */
	TWeakObjectPtr<class AMonster> Monster;
	
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTService_HealthRecovery : public UBTService_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTService_HealthRecovery();

private:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTHealthRecoveryMemory); }

private:
	/** 매 틱마다 회복되는 체력 비율 */
	UPROPERTY(EditAnywhere)
	float RecoveryRate = 0.05f;
	
};
