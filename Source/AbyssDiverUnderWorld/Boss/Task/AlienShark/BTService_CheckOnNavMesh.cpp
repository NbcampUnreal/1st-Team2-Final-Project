// [DEPRECATED] - NavMesh는 더 이상 사용되지 않습니다.
// 이 서비스는 기존 BehaviorTree와의 호환성을 위해 유지되며, 항상 true를 반환합니다.

#include "Boss/Task/AlienShark/BTService_CheckOnNavMesh.h"
#include "Boss/Boss.h"
#include "Boss/EnhancedBossAIController.h"

const FName UBTService_CheckOnNavMesh::NavMeshCheckKey = "bIsOnNavMesh";

UBTService_CheckOnNavMesh::UBTService_CheckOnNavMesh()
{
	NodeName = TEXT("[DEPRECATED] Check On NavMesh");
	bCreateNodeInstance = false;
	bNotifyBecomeRelevant = true;
	Interval = 0.5f;
}

void UBTService_CheckOnNavMesh::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	
	FBTCheckOnNavMeshMemory* TaskMemory = (FBTCheckOnNavMeshMemory*)NodeMemory;
	if (!TaskMemory) return;

	TaskMemory->AIController = Cast<AEnhancedBossAIController>(OwnerComp.GetAIOwner());
	TaskMemory->Boss = Cast<ABoss>(OwnerComp.GetAIOwner()->GetCharacter());

	if (!TaskMemory->AIController.IsValid() || !TaskMemory->Boss.IsValid()) return;
}

void UBTService_CheckOnNavMesh::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	FBTCheckOnNavMeshMemory* TaskMemory = (FBTCheckOnNavMeshMemory*)NodeMemory;
	if (!TaskMemory || !TaskMemory->AIController.IsValid() || !TaskMemory->Boss.IsValid()) return;
	
	const bool bIsOnNavMesh = TaskMemory->Boss->IsLocationOnNavMesh(TaskMemory->Boss->GetActorLocation());
	TaskMemory->AIController->GetBlackboardComponent()->SetValueAsBool(NavMeshCheckKey, bIsOnNavMesh);
}
