#include "Monster/BT/Bosses/Kraken/BTTask_BossChasing.h"

#include "AbyssDiverUnderWorld.h"

#include "Character/UnderwaterCharacter.h"
#include "Monster/Boss/Boss.h"
//#include "Monster/Boss/Enum/EBossState.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "AIController.h"

const FName UBTTask_BossChasing::bCanAttackKey = "bCanAttack";
const FName UBTTask_BossChasing::bIsChasingKey = "bIsChasing";

UBTTask_BossChasing::UBTTask_BossChasing()
{
	NodeName = TEXT("Boss Chasing");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_BossChasing::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 컨트롤러 캐스팅 실패하면 얼리 리턴
	ABossAIController* AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;

	// 보스 캐스팅 실패하면 얼리 리턴
	ABoss* Boss = Cast<ABoss>(OwnerComp.GetAIOwner()->GetCharacter());
	if (!IsValid(Boss)) return EBTNodeResult::Failed;

	// 보스의 시야각을 넓게 전환한 후 이동속도 증가
	Boss->SetMoveSpeed(MoveSpeedMultiplier);
	
	// TickTask로 전이
	return EBTNodeResult::InProgress;
}

void UBTTask_BossChasing::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	// 캐스팅
	ABossAIController* AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	ABoss* Boss = Cast<ABoss>(OwnerComp.GetAIOwner()->GetCharacter());
	
	// 추적하던 타겟이 사라진 경우 Investigate 상태로 전이
	if (!IsValid(Boss->GetTarget()))
	{
		LOG(TEXT("Target Lost"));
		AIController->GetBlackboardComponent()->SetValueAsBool(bIsChasingKey, false);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// 추적하는 타겟 방향으로 보스 이동
	AIController->MoveToActorWithRadius(Boss->GetTarget());

	// 공격 가능한 범위가 들어온 경우 **공격 상태**로 전이
	if (Boss->GetIsAttackCollisionOverlappedPlayer())
	{
		LOG(TEXT("Target In Attack Range"));
		AIController->GetBlackboardComponent()->SetValueAsBool(bCanAttackKey, true);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
}
