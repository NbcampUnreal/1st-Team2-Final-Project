#include "Boss/Task/AlienShark/BTTask_SetNavPointAroundTarget.h"
#include "NavigationSystem.h"
#include "Boss/Boss.h"
#include "Boss/EnhancedBossAIController.h"

UBTTask_SetNavPointAroundTarget::UBTTask_SetNavPointAroundTarget()
{
	NodeName = TEXT("Set NavPoint Around Target");
	bNotifyTick = false;
	bNotifyTaskFinished = false;
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBTTask_SetNavPointAroundTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AEnhancedBossAIController* AIController = Cast<AEnhancedBossAIController>(OwnerComp.GetAIOwner());
	ABoss* Boss = Cast<ABoss>(OwnerComp.GetAIOwner()->GetCharacter());
	
	if (!IsValid(AIController) || !IsValid(Boss)) return EBTNodeResult::Failed;

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSystem)
	{
		return EBTNodeResult::Failed;
	}

	// 반경 100 내에서 랜덤한 NavMesh 포인트 찾기
	FNavLocation ResultLocation;
	const FVector TargetLocation = AIController->GetBlackboardComponent()->GetValueAsVector(TargetLocationkey.SelectedKeyName);
    
	const bool bFoundPoint = NavSystem->GetRandomReachablePointInRadius(
		TargetLocation,						// 기준점
		SearchRadius,						// 검색 반경
		ResultLocation					// 결과 위치
	);
	
	const FName BlackboardKeyName = GetSelectedBlackboardKey();

	if (bFoundPoint)
	{
		AIController->GetBlackboardComponent()->SetValueAsVector(BlackboardKeyName, ResultLocation);
	}
	else
	{
		const FVector CurrentLocation = Boss->GetActorLocation();
		AIController->GetBlackboardComponent()->SetValueAsVector(BlackboardKeyName, CurrentLocation);
	}
	
	return EBTNodeResult::Succeeded;
}
