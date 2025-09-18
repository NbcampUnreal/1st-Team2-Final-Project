#include "Monster/BT/Serpmare/BTTask_SerpmareIdle.h"

#include "AbyssDiverUnderWorld.h"

#include "Monster/Serpmare/Serpmare.h"
#include "Monster/MonsterAIController.h"
#include "Character/UnderwaterCharacter.h"

#include "Components/CapsuleComponent.h"

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

	TaskMemory->AIController = Cast<AMonsterAIController>(OwnerComp.GetAIOwner());
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
			TaskMemory->Serpmare->M_PlayMontage(TaskMemory->Serpmare->AppearAnimation);
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
		else
		{
			TaskMemory->Serpmare->SetMonsterState(EMonsterState::Detected);
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);	
		}

		TArray<AActor*> OverlappedActors;
		TaskMemory->Serpmare->AttackCollision->GetOverlappingActors(OverlappedActors);

		for (AActor* OverlappedActor : OverlappedActors)
		{
			AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(OverlappedActor);
			if (IsValid(Player))
			{
				TaskMemory->Serpmare->AddDetection(Player);
				break;
			}
		}
	}
}
