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

	// ABP의 상태전이를 위한 BossState 전환 코드이다. 이동 로직에는 관여하지 않는다.
	TaskMemory->Boss->SetBossState(EBossState::Chase);

	// 최대로 플레이어를 쫓을 시간을 초기화하는 코드이다.
	// ChaseAccumulatedTime이 FinishTaskInterval을 넘어가면 추적을 중단한다.
	TaskMemory->Boss->ChaseAccumulatedTime = 0.f;
	TaskMemory->FinishTaskInterval = FMath::RandRange(MinChaseTime, MaxChaseTime);

	// 추적 상태에서는 감속을 받지 않고, 이동속도가 증가한다.
	TaskMemory->Boss->SetCharacterMovementSetting(ChaseDeceleration ,ChaseMoveSpeed);
	
	return EBTNodeResult::InProgress;
}

void UBTTask_PlayerChase::TickTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(Comp, NodeMemory, DeltaSeconds);

	FBTPlayerChaseTaskMemory* TaskMemory = (FBTPlayerChaseTaskMemory*)NodeMemory;
	if (!TaskMemory) return;
	

	// 추적중인 액터에게 MoveTo를 호출하는 코드이다.
	Result = TaskMemory->AIController->MoveToActorWithRadius(TaskMemory->Boss->GetTarget());

	// 추적 중인 플레이어가 사망 상태인 경우 추적을 중단한다.
	if (IsValid(TaskMemory->Boss->GetTarget()))
	{
		if (TaskMemory->Boss->GetTarget()->GetCharacterState() == ECharacterState::Death)
		{
			TaskMemory->AIController->InitVariables();
			return;
		}
	}
	
	// 추적하는 타겟 방향으로 이동한다.
	// 만약 추적하는 과정에서 타겟이 NavMesh를 벗어난다면 랜덤한 NavMesh 지점으로 이동한다.
	if (Result == EPathFollowingRequestResult::Failed)
	{
		LOG(TEXT("Failed to move to actor."));
		TaskMemory->AIController->SetBlackboardPerceptionType(EPerceptionType::Finish);
		return;
	}

	// 플레이어가 시야에서 사라진 경우
	if (TaskMemory->AIController->GetIsDisappearPlayer())
	{
		if (IsValid(TaskMemory->Boss->GetTarget()))
		{
			// 플레이어가 해초 더미 속에 숨은 경우
			if (TaskMemory->Boss->GetTarget()->IsHideInSeaweed())
			{
				TaskMemory->AIController->GetBlackboardComponent()->SetValueAsBool(bIsPlayerHiddenKey, true);
			}	
		}
	}

	// 정해진 시간만큼 경과하면 추적을 중단한다.
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

	// 추적 상태가 끝나는 경우 감속과 이동속도를 원래 상태로 복원한다.
	TaskMemory->Boss->InitCharacterMovementSetting();
}
