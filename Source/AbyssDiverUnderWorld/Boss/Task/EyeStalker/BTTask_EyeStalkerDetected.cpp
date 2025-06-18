#include "Boss/Task/EyeStalker/BTTask_EyeStalkerDetected.h"
#include "Boss/EyeStalker/EyeStalker.h"
#include "Boss/EyeStalker/EyeStalkerAIController.h"
#include "Character/UnderwaterCharacter.h"

UBTTask_EyeStalkerDetected::UBTTask_EyeStalkerDetected()
{
	NodeName = TEXT("EyeStalker Detected");
	bNotifyTick = true;
	bNotifyTaskFinished = false;
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBTTask_EyeStalkerDetected::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FEyeStalkerDetectedMemory* TaskMemory = (FEyeStalkerDetectedMemory*)NodeMemory;
	if (!TaskMemory) return EBTNodeResult::Failed;

	TaskMemory->AIController = Cast<AEyeStalkerAIController>(OwnerComp.GetAIOwner());
	TaskMemory->EyeStalker = Cast<AEyeStalker>(OwnerComp.GetAIOwner()->GetCharacter());
	
	if (!TaskMemory->AIController.IsValid() || !TaskMemory->EyeStalker.IsValid()) return EBTNodeResult::Failed;

	// 측정 시간 초기화
	TaskMemory->AccumulatedTime = 0.0f;

	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(TaskMemory->AIController->GetBlackboardComponent()->GetValueAsObject("TargetPlayer"));
	if (!IsValid(Player)) return EBTNodeResult::Failed;

	// EyeStalker 상태 초기화
	TaskMemory->EyeStalker->M_SetEyeOpenness(1.0f);		// 눈 뜨기
	TaskMemory->EyeStalker->M_SetTargetPlayer(Player);	// 플레이어 타겟 초기화
	TaskMemory->EyeStalker->M_SetDetectedState(true);		// 탐지 시작
	
	return EBTNodeResult::InProgress;
}

void UBTTask_EyeStalkerDetected::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	FEyeStalkerDetectedMemory* TaskMemory = (FEyeStalkerDetectedMemory*)NodeMemory;
	if (!TaskMemory) return;
	
	// @TODO: 플레어건 반경에 존재할 경우 얼리 리턴

	// 타겟이 유효하지 않거나 사망 상태인 경우 탐지 중단
	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(TaskMemory->AIController->GetBlackboardComponent()->GetValueAsObject("TargetPlayer"));
	if (!IsValid(Player) || !Player->IsAlive())
	{
		TaskMemory->AIController->GetBlackboardComponent()->SetValueAsObject("TargetPlayer", nullptr);
		TaskMemory->AIController->GetBlackboardComponent()->SetValueAsBool("bHasDetected", false);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// 플레이어가 이동 상태인지 확인하기 위해 **이동속도** 추출
	const float PlayerVelocity = Player->GetVelocity().Size();
	const bool bIsMoving = PlayerVelocity >= 5.0f;
    
	// 상태가 변경되었으면 시간 리셋
	if (bIsMoving != TaskMemory->bWasPlayerMoving)
	{
		TaskMemory->AccumulatedTime = 0.0f;
		TaskMemory->bWasPlayerMoving = bIsMoving;
	}
    
	// 시간 누적
	TaskMemory->AccumulatedTime += DeltaSeconds;
    
	// 현재 상태에 따른 완료 조건 체크
	const float RequiredTime = bIsMoving ? MoveDetectedTime : PauseDetectedTime;
    
	if (TaskMemory->AccumulatedTime >= RequiredTime)
	{
		if (bIsMoving)
		{
			TaskMemory->AIController->GetBlackboardComponent()->SetValueAsBool("bIsAttacking", true);
		}
		else
		{
			TaskMemory->AIController->GetBlackboardComponent()->SetValueAsObject("TargetPlayer", nullptr);
			TaskMemory->AIController->GetBlackboardComponent()->SetValueAsBool("bHasDetected", false);
		}
		
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
