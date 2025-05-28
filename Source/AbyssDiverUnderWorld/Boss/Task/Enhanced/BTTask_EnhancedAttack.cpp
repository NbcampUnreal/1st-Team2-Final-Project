#include "Boss/Task/Enhanced/BTTask_EnhancedAttack.h"
#include "AbyssDiverUnderWorld.h"
#include "Boss/Boss.h"
#include "Boss/Enum/EPerceptionType.h"

// ----- 기능 -----
// 1. 몬스터의 AnimInstance 추출
// 2. 몽타주 종료 이벤트에 함수 바인딩
// 3. 공격 반경 내에 플레이어가 들어온 경우 공격 몽타주 재생
// 4. 몽타주 종료 이벤트로 Finish 상태로 전이

// ----- 사용처 -----
// 플레이어 추격과 동시에 공격할 때

UBTTask_EnhancedAttack::UBTTask_EnhancedAttack()
{
	NodeName = "Enhanced Attack";
	bNotifyTick = true;
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBTTask_EnhancedAttack::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	FBTEnhancedAttackTaskMemory* TaskMemory = (FBTEnhancedAttackTaskMemory*)NodeMemory;
	if (!TaskMemory) return EBTNodeResult::Failed;

	TaskMemory->AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	TaskMemory->Boss = Cast<ABoss>(TaskMemory->AIController->GetCharacter());
	
	if (!TaskMemory->Boss.IsValid() || !TaskMemory->AIController.IsValid()) return EBTNodeResult::Failed;
	
	TaskMemory->BlackboardKeyName = GetSelectedBlackboardKey();
	
	return EBTNodeResult::InProgress;
}

void UBTTask_EnhancedAttack::TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(Comp, NodeMemory, DeltaSeconds);
	
	FBTEnhancedAttackTaskMemory* TaskMemory = (FBTEnhancedAttackTaskMemory*)NodeMemory;
	if (!TaskMemory) return;

	TaskMemory->AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	TaskMemory->Boss = Cast<ABoss>(TaskMemory->AIController->GetCharacter());
	
	if (!TaskMemory->Boss.IsValid() || !TaskMemory->AIController.IsValid()) return;
	
	if (TaskMemory->AIController->GetBlackboardComponent()->GetValueAsBool(TaskMemory->BlackboardKeyName)) return;
	
	if (TaskMemory->Boss->GetIsAttackCollisionOverlappedPlayer())
	{
		TaskMemory->Boss->Attack();
		TaskMemory->AIController->GetBlackboardComponent()->SetValueAsBool(TaskMemory->BlackboardKeyName, true);
	}
}
