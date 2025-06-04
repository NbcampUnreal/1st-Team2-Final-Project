#include "Boss/Task/AlienShark/BTService_HasObstacleAhead.h"
#include "Boss/Boss.h"
#include "Boss/EnhancedBossAIController.h"

const FName UBTService_HasObstacleAhead:: bHasObstacleAheadKey = "bHasObstacleAhead";

UBTService_HasObstacleAhead::UBTService_HasObstacleAhead()
{
	NodeName = TEXT("Has Obstacle Ahead");
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
	bCreateNodeInstance = false;
}

void UBTService_HasObstacleAhead::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	FBTHasObstacleAheadMemory* TaskMemory = (FBTHasObstacleAheadMemory*)NodeMemory;
	if (!TaskMemory) return;

	TaskMemory->AIController = Cast<AEnhancedBossAIController>(OwnerComp.GetAIOwner());
	TaskMemory->Boss = Cast<ABoss>(OwnerComp.GetAIOwner()->GetCharacter());
}

void UBTService_HasObstacleAhead::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);

	FBTHasObstacleAheadMemory* TaskMemory = (FBTHasObstacleAheadMemory*)NodeMemory;
	if (!TaskMemory || !TaskMemory->Boss.IsValid() || !TaskMemory->AIController.IsValid()) return;

	TaskMemory->AIController->GetBlackboardComponent()->SetValueAsBool(bHasObstacleAheadKey, false);
}

void UBTService_HasObstacleAhead::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	FBTHasObstacleAheadMemory* TaskMemory = (FBTHasObstacleAheadMemory*)NodeMemory;
	if (!TaskMemory || !TaskMemory->Boss.IsValid() || !TaskMemory->AIController.IsValid()) return;

	const bool bHasObstacleAhead = TaskMemory->Boss->HasObstacleAhead();
	TaskMemory->AIController->GetBlackboardComponent()->SetValueAsBool(bHasObstacleAheadKey, bHasObstacleAhead);
}
