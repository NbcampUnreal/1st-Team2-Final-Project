#include "Monster/Boss/Task/Serpmare/BTTask_SerpmareIdle.h"
#include "AbyssDiverUnderWorld.h"
#include "Monster/Boss/Enum/EBossState.h"
#include "Monster/Boss/Serpmare/Serpmare.h"

UBTTask_SerpmareIdle::UBTTask_SerpmareIdle()
{
	NodeName = TEXT("Serpmare Idle");
	bNotifyTick = true;
	bCreateNodeInstance = false;
	
	bIsBigSerpmare = false;
}

EBTNodeResult::Type UBTTask_SerpmareIdle::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTSerpmareIdleTaskMemory* TaskMemory = (FBTSerpmareIdleTaskMemory*)NodeMemory;
	if (!TaskMemory) return EBTNodeResult::Failed;

	TaskMemory->AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	TaskMemory->Serpmare = Cast<ASerpmare>(OwnerComp.GetAIOwner()->GetCharacter());

	if (!TaskMemory->AIController.IsValid() || !TaskMemory->Serpmare.IsValid()) return EBTNodeResult::Failed;
	
	return EBTNodeResult::InProgress;
}

void UBTTask_SerpmareIdle::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	FBTSerpmareIdleTaskMemory* TaskMemory = (FBTSerpmareIdleTaskMemory*)NodeMemory;
	if (!TaskMemory) return;

	// 공격 범위 내에 플레이어가 들어온 경우
	if (TaskMemory->Serpmare->GetIsAttackCollisionOverlappedPlayer())
	{
		if (bIsBigSerpmare)
		{
			TaskMemory->Serpmare->M_PlayAnimation(TaskMemory->Serpmare->AppearAnimation);
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
		else
		{
			TaskMemory->Serpmare->SetBossState(EBossState::Detected);
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);	
		}
	}
}
