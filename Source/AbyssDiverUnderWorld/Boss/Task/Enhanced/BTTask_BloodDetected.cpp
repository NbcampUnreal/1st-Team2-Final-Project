#include "Boss/Task/Enhanced/BTTask_BloodDetected.h"
#include "Boss/Boss.h"
#include "Boss/ENum/EBossState.h"

// ----- 기능 -----
// 1. AI의 이동 정지
// 2. 플레이어가 피를 흘린 위치 추출
// 3. 해당 위치를 향해 회전
// 4. 지정한 시간이 경과하면 Task 종료

// ----- 사용처 -----
// 플레이어가 흘린 피를 감지한 이후 해당 위치로 이동

UBTTask_BloodDetected::UBTTask_BloodDetected()
{
	NodeName = "Blood Detected";
	bNotifyTick = true;
	Boss = nullptr;
	AIController = nullptr;
	BloodOccurredLocation = FVector::ZeroVector;
	AccumulatedTime = 0;
	DetectedStateInterval = 2.f;
}

EBTNodeResult::Type UBTTask_BloodDetected::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	Boss = Cast<ABoss>(AIController->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;
	
	Boss->SetBossState(EBossState::Idle);

	AIController->StopMovement();

	AccumulatedTime = 0;
	
	const FName TargetLocationName = GetSelectedBlackboardKey();
	BloodOccurredLocation = AIController->GetBlackboardComponent()->GetValueAsVector(TargetLocationName);
	
	return EBTNodeResult::InProgress;
}

void UBTTask_BloodDetected::TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(Comp, NodeMemory, DeltaSeconds);

	Boss->RotationToTarget(BloodOccurredLocation);

	if (AccumulatedTime > DetectedStateInterval)
	{
		FinishLatentTask(Comp, EBTNodeResult::Succeeded);
	}

	AccumulatedTime += FMath::Clamp(DeltaSeconds, 0.f, 0.1f);
}
