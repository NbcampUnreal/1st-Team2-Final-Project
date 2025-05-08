#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_ChangeBossState.generated.h"

enum class EBossState : uint8;

UCLASS()
class ABYSSDIVERUNDERWORLD_API UBTTask_ChangeBossState : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_ChangeBossState();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|State", meta = (AllowPrivateAccess = "true"))
	EBossState BossState;

	static const FName BossStateKey;
};
