#include "Boss/Task/Limadon/BTTask_LimadonIdle.h"
#include "Boss/EBossState.h"
#include "Boss/Limadon/Limadon.h"
#include "Character/StatComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "Kismet/GameplayStatics.h"

UBTTask_LimadonIdle::UBTTask_LimadonIdle()
{
	NodeName = TEXT("Limadon Idle");
	bNotifyTick = true;
	AccumulatedTime = 0.0f;
	AttackInterval = 0.5f;
}

EBTNodeResult::Type UBTTask_LimadonIdle::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 캐스팅
	ALimadon* Limadon = Cast<ALimadon>(OwnerComp.GetAIOwner()->GetPawn());
	if (!IsValid(Limadon)) return EBTNodeResult::Failed;

	// 깨물기 성공했다면 Emissive 증가
	if (Limadon->GetIsBiteAttackSuccess())
	{
		Limadon->SetEmissiveUp();
	}
	
	return EBTNodeResult::InProgress;
}

void UBTTask_LimadonIdle::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	// 캐스팅
	ALimadon* Limadon = Cast<ALimadon>(OwnerComp.GetAIOwner()->GetPawn());

	// Limadon 체력이 일정 밑으로 떨어진 경우 플레이어 뱉음
	if (Limadon->GetStatComponent()->CurrentHealth <= Limadon->StopCaptureHealthCriteria)
	{
		Limadon->SetBossState(EBossState::Detected);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	//깨물기 성공한 경우 Attack Interval 마다 타겟에게 데미지
	if (Limadon->GetIsBiteAttackSuccess())
	{
		if (AttackInterval <= AccumulatedTime)
		{
			if (!IsValid(Limadon->GetTarget())) return;
			
			UGameplayStatics::ApplyDamage(Limadon->GetTarget(), Limadon->GetStatComponent()->AttackPower, Limadon->GetController(), Limadon, UDamageType::StaticClass());
			AccumulatedTime = 0.0f;

			// 공격 중인 타겟이 사망한 경우 Detected 상태로 전이 (Spit)
			if (Limadon->GetTarget()->GetStatComponent()->CurrentHealth <= 0)
			{
				Limadon->SetBossState(EBossState::Detected);
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
				return;
			}	
		}
		else
		{
			AccumulatedTime += DeltaSeconds;
		}
	}
	// 깨물기 실패한 경우 바로 결과 리턴
	else
	{
		Limadon->M_PlayAnimation(Limadon->HideAnimation);
		Limadon->BiteVariableInitialize();
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
}