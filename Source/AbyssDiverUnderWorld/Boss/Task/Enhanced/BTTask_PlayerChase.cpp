#include "Boss/Task/Enhanced/BTTask_PlayerChase.h"
#include "AbyssDiverUnderWorld.h"
#include "Boss/Boss.h"
#include "Boss/ENum/EBossState.h"
#include "Boss/Enum/EPerceptionType.h"
#include "Character/UnderwaterCharacter.h"

const FName UBTTask_PlayerChase::bIsPlayerHiddenKey = "bIsPlayerHidden";

UBTTask_PlayerChase::UBTTask_PlayerChase()
{
	NodeName = "Player Chase";
	bNotifyTick = true;
	bNotifyTaskFinished = true;
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

	Boss->SetBossState(EBossState::Chase);

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

	// 전방을 향해 이동
	Boss->MoveForward(DeltaSeconds);

	// 플레이어가 시야에서 사라진 경우
	if (AIController->GetIsDisappearPlayer())
	{
		// 플레이어가 해초 더미 속에 숨은 경우
		if (Boss->GetTarget()->IsHideInSeaweed())
		{
			AIController->GetBlackboardComponent()->SetValueAsBool(bIsPlayerHiddenKey, true);
		}
	}

	// 정해진 시간만큼 경과하면 추적 종료
	if (AccumulatedTime > TimeCriteria)
	{
		AIController->SetBlackboardPerceptionType(EPerceptionType::Finish);	
	}

	AccumulatedTime += FMath::Clamp(DeltaSeconds, 0.0f, 0.1f);
}

void UBTTask_PlayerChase::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	Boss->InitCurrentMoveSpeed();
}
