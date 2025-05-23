#include "Boss/Task/Enhanced/BTTask_PlayerChase.h"
#include "AbyssDiverUnderWorld.h"
#include "Boss/Boss.h"
#include "Boss/Enum/EPerceptionType.h"

UBTTask_PlayerChase::UBTTask_PlayerChase()
{
	NodeName = "Player Chase";
	bNotifyTick = true;
	Boss = nullptr;
	AIController = nullptr;
	MaxChaseTime = 20.f;
	MinChaseTime = 10.0f;
	AccumulatedTime = 0.f;
	TimeCriteria = 0.f;
}

EBTNodeResult::Type UBTTask_PlayerChase::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	Boss = Cast<ABoss>(AIController->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	// 랜덤 시간 추출
	AccumulatedTime = 0.f;
	TimeCriteria = FMath::RandRange(MinChaseTime, MaxChaseTime);
	
	return EBTNodeResult::InProgress;
}

void UBTTask_PlayerChase::TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(Comp, NodeMemory, DeltaSeconds);

	// 추적하는 타겟 방향으로 보스 이동
	AIController->MoveToActorWithRadius();

	// 정해진 시간만큼 경과하면 추적 종료
	if (AccumulatedTime > TimeCriteria)
	{
		AIController->SetBlackboardPerceptionType(EPerceptionType::Finish);	
	}
	else
	{
		AccumulatedTime += DeltaSeconds;
	}
}
