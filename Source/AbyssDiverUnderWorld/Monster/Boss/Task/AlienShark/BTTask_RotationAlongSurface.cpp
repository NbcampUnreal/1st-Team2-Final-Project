#include "Monster/Boss/Task/AlienShark/BTTask_RotationAlongSurface.h"
#include "Monster/Boss/Boss.h"
#include "Monster/Boss/EnhancedBossAIController.h"

UBTTask_RotationAlongSurface::UBTTask_RotationAlongSurface()
{
	NodeName = "Rotation Along Surface";
	bNotifyTick = true;
	bCreateNodeInstance = false;
}

EBTNodeResult::Type UBTTask_RotationAlongSurface::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTRotationAlongSurfaceMemory* TaskMemory = (FBTRotationAlongSurfaceMemory*)NodeMemory;
	if (!TaskMemory) return EBTNodeResult::Failed;

	TaskMemory->AIController = Cast<AEnhancedBossAIController>(OwnerComp.GetAIOwner());
	TaskMemory->Boss = Cast<ABoss>(OwnerComp.GetAIOwner()->GetCharacter());
	
	if (!TaskMemory->AIController.IsValid() || !TaskMemory->Boss.IsValid()) return EBTNodeResult::Failed;
	
	return EBTNodeResult::InProgress;
}

void UBTTask_RotationAlongSurface::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	FBTRotationAlongSurfaceMemory* TaskMemory = (FBTRotationAlongSurfaceMemory*)NodeMemory;
	if (!TaskMemory) return;

	//TaskMemory->Boss->SmoothMoveAlongSurface(DeltaSeconds);
}
