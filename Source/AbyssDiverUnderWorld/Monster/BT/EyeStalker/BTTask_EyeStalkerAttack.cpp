#include "Monster/BT/EyeStalker/BTTask_EyeStalkerAttack.h"

#include "Monster/Effect/PostProcessSettingComponent.h"
#include "Monster/Effect/VignetteVolume.h"

#include "Monster/EyeStalker/EyeStalker.h"
#include "Monster/EyeStalker/EyeStalkerAIController.h"

#include "Character/UnderwaterCharacter.h"
#include "Container/BlackboardKeys.h"

UBTTask_EyeStalkerAttack::UBTTask_EyeStalkerAttack()
{
	NodeName = TEXT("EyeStalker Attack");
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBTTask_EyeStalkerAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FEyeStalkerAttackMemory* TaskMemory = (FEyeStalkerAttackMemory*)NodeMemory;
	if (!TaskMemory) return EBTNodeResult::Failed;

	TaskMemory->AIController = Cast<AEyeStalkerAIController>(OwnerComp.GetAIOwner());
	TaskMemory->EyeStalker = Cast<AEyeStalker>(OwnerComp.GetAIOwner()->GetCharacter());
	
	if (!TaskMemory->AIController.IsValid() || !TaskMemory->EyeStalker.IsValid()) return EBTNodeResult::Failed;

	// 측정 시간 초기화
	TaskMemory->AccumulatedTime = 0.0f;

	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(TaskMemory->AIController->GetBlackboardComponent()->GetValueAsObject(BlackboardKeys::TargetPlayerKey));
	if (!IsValid(Player)) return EBTNodeResult::Failed;
	
	// EyeStalker 상태 초기화
	TaskMemory->EyeStalker->M_SetEyeOpenness(0.3f);		// 눈 반쯤 뜨기
	TaskMemory->EyeStalker->M_SetTargetPlayer(Player);	// 플레이어 타겟 설정
	TaskMemory->EyeStalker->M_SetDetectedState(true);	// 탐지 시작

	// 아이스토커 Whisper Sound 재생
	if (TaskMemory->EyeStalker->GetMonsterSoundComp())
	{
		TaskMemory->EyeStalker->GetMonsterSoundComp()->M_PlayChaseLoopSound();
	}
	
	return EBTNodeResult::InProgress;
}

void UBTTask_EyeStalkerAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	FEyeStalkerAttackMemory* TaskMemory = (FEyeStalkerAttackMemory*)NodeMemory;
	if (!TaskMemory) return;

	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(TaskMemory->AIController->GetBlackboardComponent()->GetValueAsObject(BlackboardKeys::TargetPlayerKey));
	if (!IsValid(Player) || Player->IsDeath() || Player->IsGroggy())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
	
	// 플레이어가 이동 상태인지 확인하기 위해 **이동속도** 추출
	const float PlayerVelocity = Player->GetVelocity().Size();
	const bool bIsMoving = PlayerVelocity >= RecognizationSpeed;
    
	// 상태가 변경되었으면 시간 리셋
	if (bIsMoving != TaskMemory->bWasPlayerMoving)
	{
		TaskMemory->AccumulatedTime = 0.0f;
		TaskMemory->bWasPlayerMoving = bIsMoving;
	}
    
	// 움직이지 않을 때만 시간 누적
	if (!bIsMoving)
	{
		TaskMemory->AccumulatedTime += DeltaSeconds;
	}

	// 3초동안 타겟이 움직이지 않을 경우 공격 상태 종료
	if (TaskMemory->AccumulatedTime >= PauseDetectedTime)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

}

void UBTTask_EyeStalkerAttack::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	FEyeStalkerAttackMemory* TaskMemory = (FEyeStalkerAttackMemory*)NodeMemory;
	if (!TaskMemory) return;

	TaskMemory->AIController->GetBlackboardComponent()->SetValueAsBool(BlackboardKeys::EyeStalker::bIsAttackingKey, false);
	TaskMemory->AIController->GetBlackboardComponent()->SetValueAsBool(BlackboardKeys::EyeStalker::bHasDetectedKey, false);
	TaskMemory->AIController->InitTargetPlayer();

	// 아이스토커 Whisper Sound 정지
	if (TaskMemory->EyeStalker->GetMonsterSoundComp())
	{
		TaskMemory->EyeStalker->GetMonsterSoundComp()->M_StopAllLoopSound();
	}
}
