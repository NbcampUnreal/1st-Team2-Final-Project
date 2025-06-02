#include "Boss/Task/Serpmare/BTTask_SerpmareDetected.h"
#include "Boss/Enum/EBossState.h"
#include "Boss/Serpmare/Serpmare.h"

UBTTask_SerpmareDetected::UBTTask_SerpmareDetected()
{
	NodeName = TEXT("Serpmare Detected");
	bNotifyTick = true;
	bCreateNodeInstance = false;

	AttackInterval = 3.0f;
	DetectInterval = 2.0f;
	bIsBigSerpmare = false;
}

EBTNodeResult::Type UBTTask_SerpmareDetected::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTSerpmareDetectedTaskMemory* TaskMemory = (FBTSerpmareDetectedTaskMemory*)NodeMemory;
	if (!TaskMemory) return EBTNodeResult::Failed;
	
	TaskMemory->AccumulatedAttackTime = 0.0f;
	TaskMemory->AccumulatedDetectTime = 0.0f;
	
	return EBTNodeResult::InProgress;
}

void UBTTask_SerpmareDetected::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	FBTSerpmareDetectedTaskMemory* TaskMemory = (FBTSerpmareDetectedTaskMemory*)NodeMemory;
	if (!TaskMemory) return;

	TaskMemory->AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	TaskMemory->Serpmare = Cast<ASerpmare>(TaskMemory->AIController->GetCharacter());

	if (!TaskMemory->AIController.IsValid() || !TaskMemory->Serpmare.IsValid()) return;
	
	// 플레이어가 공격 범위 내에 있는 경우
	if (TaskMemory->Serpmare->GetIsAttackCollisionOverlappedPlayer())
	{
		if (TaskMemory->AccumulatedAttackTime >= AttackInterval)
		{
			TaskMemory->Serpmare->SetBossState(EBossState::Attack);
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
		else
		{
			TaskMemory->AccumulatedAttackTime += DeltaSeconds;
		}	
	}
	// 플레이어가 공격 범위에서 벗어난 경우
	else
	{
		if (TaskMemory->AccumulatedDetectTime >= DetectInterval)
		{
			if (bIsBigSerpmare)
			{
				TaskMemory->Serpmare->M_PlayAnimation(TaskMemory->Serpmare->DisappearAnimation);
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			}
			else
			{
				TaskMemory->Serpmare->SetBossState(EBossState::Idle);
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);	
			}
		}
		else
		{
			TaskMemory->AccumulatedDetectTime += DeltaSeconds;
		}
	}
}