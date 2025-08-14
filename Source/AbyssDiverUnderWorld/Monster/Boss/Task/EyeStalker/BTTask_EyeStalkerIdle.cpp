#include "Monster/Boss/Task/EyeStalker/BTTask_EyeStalkerIdle.h"
#include "AbyssDiverUnderWorld.h"
#include "Monster/Boss/EyeStalker/EyeStalker.h"
#include "Monster/Boss/EyeStalker/EyeStalkerAIController.h"
#include "Character/UnderwaterCharacter.h"

UBTTask_EyeStalkerIdle::UBTTask_EyeStalkerIdle()
{
	NodeName = TEXT("EyeStalker Idle");
	bNotifyTick = true;
	bNotifyTaskFinished = false;
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBTTask_EyeStalkerIdle::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FEyeStalkerIdleMemory* TaskMemory = (FEyeStalkerIdleMemory*)NodeMemory;
	if (!TaskMemory) return EBTNodeResult::Failed;

	TaskMemory->AIController = Cast<AEyeStalkerAIController>(OwnerComp.GetAIOwner());
	TaskMemory->EyeStalker = Cast<AEyeStalker>(OwnerComp.GetAIOwner()->GetCharacter());

	if (!TaskMemory->AIController.IsValid() || !TaskMemory->EyeStalker.IsValid()) return EBTNodeResult::Failed;

	// EyeStalker 상태 초기화
	TaskMemory->EyeStalker->M_SetEyeOpenness(0.0f);		// 눈 감기
	TaskMemory->EyeStalker->M_SetTargetPlayer(nullptr);	// 플레이어 타겟 초기화
	TaskMemory->EyeStalker->M_SetDetectedState(false);	// 탐지 중단
	
	return EBTNodeResult::InProgress;
}

void UBTTask_EyeStalkerIdle::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	FEyeStalkerIdleMemory* TaskMemory = (FEyeStalkerIdleMemory*)NodeMemory;
	if (!TaskMemory) return;
	
	// TargetPlayers 전체를 대상으로 이동속도 확인
	for (AUnderwaterCharacter* TargetPlayer : TaskMemory->AIController->GetTargetPlayers())
	{
		if (!IsValid(TargetPlayer) || TargetPlayer->IsDeath() || TargetPlayer->IsGroggy()) continue;
		
		const float PlayerVelocity = TargetPlayer->GetVelocity().Size();
		if (PlayerVelocity >= 5.0f)
		{
			// 이동 중인 플레이어를 타겟으로 설정
			TaskMemory->AIController->GetBlackboardComponent()->SetValueAsObject("TargetPlayer", TargetPlayer);
			TaskMemory->AIController->GetBlackboardComponent()->SetValueAsBool("bHasDetected", true);
			break;
		}
	}
}

