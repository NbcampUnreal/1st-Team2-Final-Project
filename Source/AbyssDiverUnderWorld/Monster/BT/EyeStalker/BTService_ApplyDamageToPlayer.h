#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"

#include "BTService_ApplyDamageToPlayer.generated.h"

class AMonster;
class AUnderwaterCharacter;

struct FApplyDamageToPlayerMemory
{
	TWeakObjectPtr<AMonster> Monster;
	TWeakObjectPtr<class AMonsterAIController> AIController;
	uint8 bIsDamageApplied : 1 = false;
	TWeakObjectPtr<AUnderwaterCharacter> ChasingPlayer;
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

	// 이동 중이라고 인식할 정도의 스피드, BTTask_EyeStalkerAttack와 일치시키는 편이 좋다.
	UPROPERTY(EditAnywhere)
	float RecognizationSpeed = 5.0f;

	TMap<TWeakObjectPtr<AUnderwaterCharacter>, TWeakObjectPtr<AMonster>> AttackerByVictimMap;
	
};
