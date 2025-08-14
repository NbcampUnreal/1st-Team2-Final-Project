#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_ApplyDamageToPlayer.generated.h"

struct FApplyDamageToPlayerMemory
{
	TWeakObjectPtr<class ABoss> Boss;
	TWeakObjectPtr<class AEyeStalkerAIController> AIController;
	uint8 bIsDamageApplied : 1 = false;
};

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTService_ApplyDamageToPlayer : public UBTService_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTService_ApplyDamageToPlayer();

private:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FApplyDamageToPlayerMemory); }

private:
	UPROPERTY(EditAnywhere)
	float AttackPower = 200.0f;
	
};
