#include "Boss/Task/Enhanced/BTTask_PlayerChase.h"
#include "AbyssDiverUnderWorld.h"
#include "Boss/Boss.h"

UBTTask_PlayerChase::UBTTask_PlayerChase()
{
	NodeName = "Player Chase";
	bNotifyTick = true;
	Boss = nullptr;
	AIController = nullptr;
	MaxChaseTime = 10.f;
	AccumulatedTime = 0.f;
}

EBTNodeResult::Type UBTTask_PlayerChase::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	Boss = Cast<ABoss>(AIController->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	AccumulatedTime = 0.f;
	
	return EBTNodeResult::InProgress;
}

void UBTTask_PlayerChase::TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(Comp, NodeMemory, DeltaSeconds);

	// 추적하는 타겟 방향으로 보스 이동
	AIController->MoveToActorWithRadius();

	if (AccumulatedTime > MaxChaseTime)
	{
		
	}
	else
	{
		AccumulatedTime += DeltaSeconds;
	}
}
