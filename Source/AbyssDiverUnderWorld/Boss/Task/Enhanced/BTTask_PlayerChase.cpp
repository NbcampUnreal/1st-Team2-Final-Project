#include "Boss/Task/Enhanced/BTTask_PlayerChase.h"
#include "AbyssDiverUnderWorld.h"
#include "Boss/Boss.h"
#include "Boss/ENum/EBossState.h"
#include "Boss/Enum/EPerceptionType.h"
#include "Character/UnderwaterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

const FName UBTTask_PlayerChase::bIsPlayerHiddenKey = "bIsPlayerHidden";

UBTTask_PlayerChase::UBTTask_PlayerChase()
{
	NodeName = "Player Chase";
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	bCreateNodeInstance = false;
	
	MaxChaseTime = 20.f;
	MinChaseTime = 10.0f;
	ChaseDeceleration = 0.0f;
	ChaseMoveSpeed = 900.0f;
}

EBTNodeResult::Type UBTTask_PlayerChase::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
 	FBTPlayerChaseTaskMemory* TaskMemory = (FBTPlayerChaseTaskMemory*)NodeMemory;
	if (!TaskMemory) return EBTNodeResult::Failed;

	TaskMemory->AIController = Cast<AEnhancedBossAIController>(Comp.GetAIOwner());
	TaskMemory->Boss = Cast<ABoss>(TaskMemory->AIController->GetCharacter());
	
	if (!TaskMemory->Boss.IsValid() || !TaskMemory->AIController.IsValid()) return EBTNodeResult::Failed;
	
	TaskMemory->Boss->SetBossState(EBossState::Chase);
	
	TaskMemory->Boss->ChaseAccumulatedTime = 0.f;
	TaskMemory->FinishTaskInterval = FMath::RandRange(MinChaseTime, MaxChaseTime);

	TaskMemory->Boss->SetCharacterMovementSetting(ChaseDeceleration ,ChaseMoveSpeed);
	
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
	
	// 추적하는 타겟 방향으로 이동한다.
	// 만약 추적하는 과정에서 타겟이 NavMesh를 벗어난다면 즉시 스폰 위치 주변으로 이동한다.
	const EPathFollowingRequestResult::Type Result = TaskMemory->AIController->MoveToActorWithRadius();
	
	if (Result == EPathFollowingRequestResult::Failed)
	{
		LOG(TEXT("Failed to move to actor. Moving to spawn location instead."));
		TaskMemory->AIController->SetBlackboardPerceptionType(EPerceptionType::Finish);
		return;
	}

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
	if (TaskMemory->Boss->ChaseAccumulatedTime > TaskMemory->FinishTaskInterval)
	{
		TaskMemory->AIController->SetBlackboardPerceptionType(EPerceptionType::Finish);	
	}

	TaskMemory->Boss->ChaseAccumulatedTime += FMath::Clamp(DeltaSeconds, 0.0f, 0.1f);
}

void UBTTask_PlayerChase::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	FBTPlayerChaseTaskMemory* TaskMemory = (FBTPlayerChaseTaskMemory*)NodeMemory;
	if (!TaskMemory) return;

	TaskMemory->AIController = Cast<AEnhancedBossAIController>(OwnerComp.GetAIOwner());
	TaskMemory->Boss = Cast<ABoss>(TaskMemory->AIController->GetCharacter());
	
	if (!TaskMemory->Boss.IsValid() || !TaskMemory->AIController.IsValid()) return;
	
	TaskMemory->Boss->InitCharacterMovementSetting();
}
