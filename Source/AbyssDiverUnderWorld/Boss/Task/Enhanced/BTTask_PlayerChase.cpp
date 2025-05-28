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
	bCreateNodeInstance = false;
	
	MaxChaseTime = 20.f;
	MinChaseTime = 10.0f;
}

EBTNodeResult::Type UBTTask_PlayerChase::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
 	FBTPlayerChaseTaskMemory* TaskMemory = (FBTPlayerChaseTaskMemory*)NodeMemory;
	if (!TaskMemory) return EBTNodeResult::Failed;

	TaskMemory->AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	TaskMemory->Boss = Cast<ABoss>(TaskMemory->AIController->GetCharacter());
	
	if (!TaskMemory->Boss.IsValid() || !TaskMemory->AIController.IsValid()) return EBTNodeResult::Failed;
	
	TaskMemory->Boss->SetBossState(EBossState::Chase);
	
	TaskMemory->AccumulatedTime = 0.f;
	TaskMemory->FinishTaskInterval = FMath::RandRange(MinChaseTime, MaxChaseTime);
	
	return EBTNodeResult::InProgress;
}

void UBTTask_PlayerChase::TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(Comp, NodeMemory, DeltaSeconds);

	FBTPlayerChaseTaskMemory* TaskMemory = (FBTPlayerChaseTaskMemory*)NodeMemory;
	if (!TaskMemory) return;

	TaskMemory->AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	TaskMemory->Boss = Cast<ABoss>(TaskMemory->AIController->GetCharacter());
	
	if (!TaskMemory->Boss.IsValid() || !TaskMemory->AIController.IsValid()) return;

	// 추적 중인 플레이어가 사망 상태인 경우 상태 초기화
	if (IsValid(TaskMemory->Boss->GetTarget()))
	{
		if (TaskMemory->Boss->GetTarget()->GetCharacterState() == ECharacterState::Death)
		{
			TaskMemory->AIController->InitVariables();
			return;
		}
	}

	// 추적하는 타겟 방향으로 보스 이동
	TaskMemory->AIController->MoveToActorWithRadius();

	// 플레이어가 시야에서 사라진 경우
	if (TaskMemory->AIController->GetIsDisappearPlayer())
	{
		// 타겟이 유효한지 확인
		if (IsValid(TaskMemory->Boss->GetTarget()))
		{
			// 플레이어가 해초 더미 속에 숨은 경우
			if (TaskMemory->Boss->GetTarget()->IsHideInSeaweed())
			{
				TaskMemory->AIController->GetBlackboardComponent()->SetValueAsBool(bIsPlayerHiddenKey, true);
			}	
		}
	}

	// 정해진 시간만큼 경과하면 추적 종료
	if (TaskMemory->AccumulatedTime > TaskMemory->FinishTaskInterval)
	{
		TaskMemory->AIController->SetBlackboardPerceptionType(EPerceptionType::Finish);	
	}

	TaskMemory->AccumulatedTime += FMath::Clamp(DeltaSeconds, 0.0f, 0.1f);
}
