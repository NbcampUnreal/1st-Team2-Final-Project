#include "Boss/Task/Serpmare/BTTask_SerpmareAttack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Boss/Enum/EBossState.h"
#include "Boss/Serpmare/Serpmare.h"

UBTTask_SerpmareAttack::UBTTask_SerpmareAttack()
{
	NodeName = TEXT("Serpmare Attack");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_SerpmareAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 캐스팅
	ABossAIController* AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Failed;
	
	ASerpmare* Serpmare = Cast<ASerpmare>(AIController->GetCharacter());
	if (!IsValid(Serpmare)) return EBTNodeResult::Failed;

	// 공격 애니메이션 재생
	Serpmare->Attack();
	
	return EBTNodeResult::InProgress;
}

void UBTTask_SerpmareAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	// 캐스팅
	ABossAIController* AIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());

	if (!AIController->IsStateSame(EBossState::Attack))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}