#include "Boss/Task/Serpmare/BTTask_SerpmareDetected.h"

#include "Boss/EBossState.h"
#include "Boss/Serpmare/Serpmare.h"

UBTTask_SerpmareDetected::UBTTask_SerpmareDetected()
{
	NodeName = TEXT("Serpmare Detected");
	bNotifyTick = true;

	AttackInterval = 3.0f;
	AccumulatedAttackTime = 0.0f;

	DetectInterval = 2.0f;
	AccumulatedDetectTime = 0.0f;
}

EBTNodeResult::Type UBTTask_SerpmareDetected::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return EBTNodeResult::InProgress;
}

void UBTTask_SerpmareDetected::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
	
	// 캐스팅
	ASerpmare* Serpmare = Cast<ASerpmare>(OwnerComp.GetAIOwner()->GetCharacter());

	//@TODO: 감지한 플레이어 방향으로 회전

	// 플레이어가 공격 범위 내에 있는 경우
	if (Serpmare->GetIsAttackCollisionOverlappedPlayer())
	{
		if (AccumulatedAttackTime >= AttackInterval)
		{
			Serpmare->SetBossState(EBossState::Attack);
			AccumulatedAttackTime = 0.0f;
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
		else
		{
			AccumulatedAttackTime += DeltaSeconds;
		}	
	}
	// 플레이어가 공격 범위에서 벗어난 경우
	else
	{
		if (AccumulatedDetectTime >= DetectInterval)
		{
			Serpmare->SetBossState(EBossState::Idle);
			AccumulatedDetectTime = 0.0f;
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
		else
		{
			AccumulatedDetectTime += DeltaSeconds;
		}
	}
}