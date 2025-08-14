#include "Monster/Boss/Task/Limadon/BTTask_LimadonIdle.h"
#include "Monster/Boss/Enum/EBossState.h"
#include "Monster/Boss/Limadon/Limadon.h"
#include "Character/StatComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "Kismet/GameplayStatics.h"

UBTTask_LimadonIdle::UBTTask_LimadonIdle()
{
	NodeName = TEXT("Limadon Idle");
	bNotifyTick = true;
	bCreateNodeInstance = false;
	
	AttackInterval = 0.5f;
}

EBTNodeResult::Type UBTTask_LimadonIdle::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTLimadonIdleTaskMemory* TaskMemory = (FBTLimadonIdleTaskMemory*)NodeMemory;
	if (!TaskMemory) return EBTNodeResult::Failed;
	
	TaskMemory->AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	TaskMemory->Limadon = Cast<ALimadon>(TaskMemory->AIController->GetCharacter());
	
	if (!TaskMemory->Limadon.IsValid() || !TaskMemory->AIController.IsValid()) return EBTNodeResult::Failed;

	// 깨물기 성공했다면 Emissive 증가
	if (TaskMemory->Limadon->GetIsBiteAttackSuccess())
	{
		TaskMemory->Limadon->SetEmissiveUp();
	}
	
	return EBTNodeResult::InProgress;
}

void UBTTask_LimadonIdle::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	FBTLimadonIdleTaskMemory* TaskMemory = (FBTLimadonIdleTaskMemory*)NodeMemory;
	if (!TaskMemory) return;
	
	TaskMemory->AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	TaskMemory->Limadon = Cast<ALimadon>(TaskMemory->AIController->GetCharacter());
	
	if (!TaskMemory->Limadon.IsValid() || !TaskMemory->AIController.IsValid()) return;

	// Limadon 체력이 일정 밑으로 떨어진 경우 플레이어 뱉음
	if (TaskMemory->Limadon->GetStatComponent()->CurrentHealth <= TaskMemory->Limadon->StopCaptureHealthCriteria)
	{
		TaskMemory->Limadon->SetBossState(EBossState::Detected);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	//깨물기 성공한 경우 Attack Interval 마다 타겟에게 데미지
	if (TaskMemory->Limadon->GetIsBiteAttackSuccess())
	{
		if (AttackInterval <= TaskMemory->AccumulatedTime)
		{
			if (!IsValid(TaskMemory->Limadon->GetTarget())) return;
			
			UGameplayStatics::ApplyDamage(TaskMemory->Limadon->GetTarget(), TaskMemory->Limadon->GetStatComponent()->AttackPower, TaskMemory->Limadon->GetController(), TaskMemory->Limadon.Get(), UDamageType::StaticClass());
			TaskMemory->AccumulatedTime = 0.0f;

			// 공격 중인 타겟이 사망한 경우 Detected 상태로 전이 (Spit)
			if (TaskMemory->Limadon->GetTarget()->GetStatComponent()->CurrentHealth <= 0)
			{
				TaskMemory->Limadon->SetBossState(EBossState::Detected);
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
				return;
			}	
		}
		else
		{
			TaskMemory->AccumulatedTime += DeltaSeconds;
		}
	}
	// 깨물기 실패한 경우 바로 결과 리턴
	else
	{
		TaskMemory->Limadon->M_PlayAnimation(TaskMemory->Limadon->HideAnimation);
		TaskMemory->Limadon->BiteVariableInitialize();
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
}