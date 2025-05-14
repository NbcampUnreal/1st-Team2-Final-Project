#include "Boss/Task/BTTask_Detected.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Boss/Boss.h"
#include "Boss/EBossState.h"

const FName UBTTask_Detected::BossStateKey = "BossState";

UBTTask_Detected::UBTTask_Detected()
{
	NodeName = TEXT("Detected Target");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_Detected::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 컨트롤러 캐스팅에 실패하면 얼리 리턴
	ABossAIController* AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	// 보스 캐스팅에 실패하면 얼리 리턴
	ABoss* Boss = Cast<ABoss>(AIController->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	// 보스의 움직임을 멈춘 후 Detected 애니메이션 재생
	AIController->StopMovement();
	AIController->SetVisionAngle(AIController->ChasingVisionAngle);
	Boss->M_PlayAnimation(Boss->DetectedAnimation);

	// TickTask로 전이
	return EBTNodeResult::InProgress;
}

void UBTTask_Detected::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	// 캐스팅
	ABoss* Boss = Cast<ABoss>(OwnerComp.GetAIOwner()->GetCharacter());
	
	// 타겟 방향으로 보스 회전
	Boss->RotationToTarget(Boss->GetTarget());
	
	// Detected 상태가 끝난 경우 결과 반환
	if (static_cast<EBossState>(OwnerComp.GetBlackboardComponent()->GetValueAsEnum(BossStateKey)) != EBossState::Detected)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
}
