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
	bNotifyTaskFinished = true;
	Boss = nullptr;
	AIController = nullptr;
}

EBTNodeResult::Type UBTTask_EnhancedAttack::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	Boss = Cast<ABoss>(AIController->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;
	
	Boss->SetDecelerate(true);

	BlackboardKeyName = GetSelectedBlackboardKey();
	
	AnimInstance = Boss->GetAnimInstance();
	AnimInstance->OnMontageEnded.AddDynamic(this, &UBTTask_EnhancedAttack::FinishPerception);
	
	return EBTNodeResult::InProgress;
}

void UBTTask_EnhancedAttack::TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(Comp, NodeMemory, DeltaSeconds);
	
	if (AIController->GetBlackboardComponent()->GetValueAsBool(BlackboardKeyName)) return;
	
	if (Boss->GetIsAttackCollisionOverlappedPlayer())
	{
		Boss->Attack();
		AIController->GetBlackboardComponent()->SetValueAsBool(BlackboardKeyName, true);
	}
}

void UBTTask_EnhancedAttack::FinishPerception(UAnimMontage* Montage, bool bInterrupted)
{
	if (IsValid(AIController))
	{
		AIController->GetBlackboardComponent()->SetValueAsBool("bHasDetectedPlayer", false);
		AIController->SetBlackboardPerceptionType(EPerceptionType::Finish);	
	}

	Boss->SetDecelerate(false);
	
	// 바인딩 해제 (중복 방지)
	if (IsValid(AnimInstance))
	{
		AnimInstance->OnMontageEnded.RemoveDynamic(this, &UBTTask_EnhancedAttack::FinishPerception);
	}

}
