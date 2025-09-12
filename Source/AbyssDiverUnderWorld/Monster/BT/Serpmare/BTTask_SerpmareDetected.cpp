#include "Monster/BT/Serpmare/BTTask_SerpmareDetected.h"

#include "Monster/Serpmare/Serpmare.h"
#include "Monster/MonsterAIController.h"

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

	TaskMemory->AIController = Cast<AMonsterAIController>(OwnerComp.GetAIOwner());
	TaskMemory->Serpmare = Cast<ASerpmare>(OwnerComp.GetAIOwner()->GetCharacter());
	
	if (!TaskMemory->AIController.IsValid() || !TaskMemory->Serpmare.IsValid()) return EBTNodeResult::Failed;
	
	TaskMemory->AccumulatedDetectTime = 0.0f;
	
	return EBTNodeResult::InProgress;
}

void UBTTask_SerpmareDetected::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	FBTSerpmareDetectedTaskMemory* TaskMemory = (FBTSerpmareDetectedTaskMemory*)NodeMemory;
	if (!TaskMemory) return;
	
	// 플레이어가 공격 범위 내에 있는 경우
	if (TaskMemory->Serpmare->GetIsAttackCollisionOverlappedPlayer())
	{
		if (TaskMemory->Serpmare->GetCanAttack())
		{
			TaskMemory->Serpmare->SetMonsterState(EMonsterState::Attack);
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
	}
	// 플레이어가 공격 범위에서 벗어난 경우
	else
	{
		if (TaskMemory->AccumulatedDetectTime >= DetectInterval)
		{
			if (bIsBigSerpmare)
			{
				TaskMemory->Serpmare->M_PlayMontage(TaskMemory->Serpmare->DisappearAnimation);
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			}
			else
			{
				TaskMemory->Serpmare->SetMonsterState(EMonsterState::Idle);
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);	
			}

			TaskMemory->Serpmare->ForceRemoveDetectedPlayers();
		}
		else
		{
			TaskMemory->AccumulatedDetectTime += DeltaSeconds;
		}
	}
}